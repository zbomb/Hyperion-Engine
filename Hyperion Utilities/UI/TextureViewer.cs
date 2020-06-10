using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Runtime.Remoting.Messaging;
using System.Runtime.Remoting.Metadata.W3cXsd2001;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Runtime.InteropServices;

namespace Hyperion
{

	public partial class TextureViewer : Form
	{
		// Fields
		private Texture m_CurrentTexture;
		private byte[] m_ImagePreviewData;
		private int m_PreviewWidth;
		private int m_PreviewHeight;
		private float m_ZoomValue;
		private string m_TexturePath;
		private int m_PreviewOffsetX;
		private int m_PreviewOffsetY;

		private Bitmap m_PreviewBuffer;
		private ZoomControl m_ZoomControl;

		private bool m_Dragging;
		private Point m_DragBegin;

		private static readonly int MinTextureFileSizeBytes = 37;
		private static readonly float MinZoomValue = 0.01f;
		private static readonly float MaxZoomValue = 100.0f;


		/*----------------------------------------------------------------------------------------
			TextureViewer.TextureViewer() [Constructor]
			* Creates and initializes the window
		----------------------------------------------------------------------------------------*/
		public TextureViewer()
		{
			m_CurrentTexture = null;
			m_ImagePreviewData = null;
			m_PreviewWidth = 0;
			m_PreviewHeight = 0;
			m_ZoomValue = 1.0f;
			m_TexturePath = null;
			m_Dragging = false;
			m_DragBegin = Point.Empty;
			m_PreviewOffsetX = 0;
			m_PreviewOffsetY = 0;

			InitializeComponent();

			// Create zoom control
			m_ZoomControl = new ZoomControl( MinZoomValue, MaxZoomValue, 1.0f, 0.1f );
			previewPanel.Controls.Add( m_ZoomControl );
			m_ZoomControl.OnValueChanged += OnZoomChanged;

			ResetUI();
			PositionZoomControl();
		}


		private void PositionZoomControl()
		{
			if( m_ZoomControl != null )
			{
				var newX = previewPanel.Width - m_ZoomControl.Width - 8;
				var newY = previewPanel.Height - m_ZoomControl.Height - 8;

				m_ZoomControl.Location = new Point( newX, newY );
				m_ZoomControl.BringToFront();
			}
		}


		/*----------------------------------------------------------------------------------------
			TextureViewer.SizeInBytesToString
			* Helper function, used to convert a size in bytes, to a readable string
		----------------------------------------------------------------------------------------*/
		private string SizeInBytesToString( long inBytes )
		{
			const long kilo = 1024;
			const long mega = kilo * 1024;
			const long giga = mega * 1024;

			// We will go from bytes => kb => mb => gb
			if( inBytes < kilo )
			{
				return inBytes.ToString() + ( inBytes == 1 ? " byte" : " bytes" );
			}
			else if( inBytes < mega )
			{
				return( inBytes / ( float ) kilo ).ToString( "0.00" ) + "KB";
			}
			else if( inBytes < giga )
			{
				return( inBytes / ( float ) mega ).ToString( "0.00" ) + "MB";
			}
			else
			{
				return ( inBytes / ( float ) giga ).ToString( "0.00" ) + "GB";
			}
		}

		private string SizeInBytesToString( int inBytes )
		{
			return inBytes >= 0 ? SizeInBytesToString( ( long ) inBytes ) : "0 bytes";
		}


		/*----------------------------------------------------------------------------------------
			TextureViewer.ResetUI()
			* Resets the UI so everything is how it is when the window is loaded
		----------------------------------------------------------------------------------------*/
		private void ResetUI()
		{
			// Clear the open texture data, so when we change the lod thats selected, it doesnt force a redraw
			m_CurrentTexture = null;
			m_ImagePreviewData = null;
			m_PreviewWidth = 0;
			m_PreviewHeight = 0;
			m_TexturePath = null;
			m_Dragging = false;
			m_DragBegin = Point.Empty;
			m_PreviewOffsetX = 0;
			m_PreviewOffsetY = 0;

			// Clear the LOD list
			lodList.Items.Clear();
			lodList.SelectedIndex = -1;
			lodList.SelectedItem = null;

			// Clear the file selection box
			fileBox.Text = "Select File...";

			// Clear the labels
			texSizeLabel.Text = "Full Size: 0x0\n0 bytes";
			lodSizeLabel.Text = "LOD Size: 0x0\n0 bytes";
			formatLabel.Text = "Format: N/A";
			cursorLabel.Text = "Cursor: N/A";

			// Enable browse button, disable clear button
			browseButton.Enabled = true;
			clearButton.Enabled = false;

			// Reset the window title
			this.Text = "Texture Viewer";

			// Disable the zoom control
			m_ZoomControl.Enabled = false;

			// Ensure the preview gets cleared
			DrawImagePreview( true );
		}


		/*----------------------------------------------------------------------------------------
			TextureViewer.OpenTexture( string )
			* Opens a texture file into the viewer
		----------------------------------------------------------------------------------------*/
		public bool OpenTexture( string inFile )
		{
			// Check if the parameter is valid, and directs us to a valid hyperion texture file
			if( ( inFile?.Length ?? 0 ) < 3 || !File.Exists( inFile ) || !inFile.ToLowerInvariant().EndsWith( ".htx" ) )
			{
				Core.WriteLine( "[Warning] TextureViewer: Failed to open file (", inFile, ") because either the file doesnt exist, or its not the proper extension (.htx)" );
				return false;
			}

			// Next, we need to read the file data in
			byte[] fileData = null;

			try
			{
				using( var fStream = new FileStream( inFile, FileMode.Open, FileAccess.Read ) )
				{
					if( fStream.CanSeek ) { fStream.Seek( 0, SeekOrigin.Begin ); }

					fileData = new byte[ fStream.Length ];
					var toRead = (int) fStream.Length;
					int read = 0;

					while( toRead > 0 )
					{
						var res = fStream.Read( fileData, read, toRead );
						if( res == 0 ) { break; } // break for end of file

						toRead -= res;
						read += res;
					}
				}

				if( fileData.Length < MinTextureFileSizeBytes )
				{
					throw new Exception( "The file didnt meet the minimum size for a hyperion texture" );
				}
			}
			catch( Exception Ex )
			{
				Core.WriteLine( "[Warning] TextureViewer: Failed to open texture (", inFile, ") because it couldnt be read [", Ex.Message, "]" );
				return false;
			}

			// Now, we need to use the texture manager to actually convert the data we have into a texture object
			// Since were uinsg the 'in-memory' version of loading textures, the texture object wont be assigned a file path, or asset id by the texture manager
			var newTexture = TextureManager.ReadTextureFromMemory( fileData );
			if( newTexture == null || newTexture.LODs.Length <= 0 )
			{
				Core.WriteLine( "[Warning] TextureViewer: Failed to open texture (", inFile, ") because it couldnt be read (or had no data)" );
				return false;
			}

			m_TexturePath = inFile;
			this.Text = "Texture Viewer [" + inFile + "]";

			// Check if we have an existing texture
			if( m_CurrentTexture != null )
			{
				// IN FUTURE: Potentially prompt the user to save work, if we ever end up allowing the editing of textures in some way
				m_CurrentTexture = null;
			}

			// Now, start to setup the UI for this new texture
			m_CurrentTexture = newTexture;
			UpdateUI();

			return true;
		}


		private byte GetSelectedLODIndex()
		{
			return (byte)( lodList.SelectedIndex > 0 ? lodList.SelectedIndex : 0 );
		}


		/*----------------------------------------------------------------------------------------
			TextureViewer.UpdateUI()
			* Called after a change is made in the current texture, sets control values
			  to reflect the currently loaded texture
		----------------------------------------------------------------------------------------*/
		private void UpdateUI()
		{
			// First, ensure there is a texture set, otherwise we will reset the UI
			if( m_CurrentTexture == null )
			{
				ResetUI();
				return;
			}

			// First, lets start with the labels and the file box
			fileBox.Text = m_TexturePath ?? "[Invalid Path]";
			formatLabel.Text = "Format: \n" + Enum.GetName( typeof( TextureFormat ), m_CurrentTexture.Format );

			browseButton.Enabled = false;
			clearButton.Enabled = true;
			
			if( ( m_CurrentTexture.LODs?.Length ?? 0 ) == 0 )
			{
				// If there are no LODs to display...
				texSizeLabel.Text	= "Full Size: N/A\n0 bytes [NO LODS]";
				lodSizeLabel.Text	= "LOD Size: N/A\n0 bytes [NO LODS]";
				cursorLabel.Text	= "Cursor: N/A";
				m_ZoomControl.Enabled = false;

				// Selecting an invalid index, should clear out the image preview
				lodList.Items.Clear();
				lodList.SelectedIndex = -1;
			}
			else
			{
				// Setup LOD list 
				for( int i = 0; i < m_CurrentTexture.LODs.Length; i++ )
				{
					lodList.Items.Add( i );
				}

				// If the texture has LODs like it should...
				// First, calculate texture full pixel data size
				long totalSize = 0L;
				foreach( var l in m_CurrentTexture.LODs )
				{
					totalSize += ( l.Data?.Length ?? 0 );
				}

				uint fullW = m_CurrentTexture.LODs[ 0 ].Width;
				uint fullH = m_CurrentTexture.LODs[ 0 ].Height;

				texSizeLabel.Text = "Full Size: " + fullW + "x" + fullH + " \n" + SizeInBytesToString( totalSize );

				// Automatically select LOD 0
				// This should cause the rest of the UI to update in response
				lodList.SelectedIndex = 0;
			}
		}


		/*----------------------------------------------------------------------------------------
			TextureViewer.UpdateCursorLabel()
			* Called from various events, this function will update the cursor label
			* This includes where the cursor is in texture space of the current LOD, and
			  also the color of the currently selected pixel
		----------------------------------------------------------------------------------------*/
		private void UpdateCursorLabel()
		{
			// Check if there is a valid perview to display
			var index = GetSelectedLODIndex();
			if( ( m_CurrentTexture?.LODs?.Length ?? 0 ) <= index ||
				( m_ImagePreviewData?.Length ?? 0 ) < 4 )
			{
				cursorLabel.Text = "Cursor: N/A \n";
				return;
			}

			// We need to calculate the position of the mouse in 'image-space'
			// We can do this by getting the position in 'panel-space' relative to the image preview panel,
			// then, use the scaling factor to calcualte the approximate position in the source image
			var clientPos = imagePanel.PointToClient( MousePosition );

			// Clamp to range
			clientPos.X = Math.Max( 0, Math.Min( imagePanel.ClientSize.Width, clientPos.X ) );
			clientPos.Y = Math.Max( 0, Math.Min( imagePanel.ClientSize.Height, clientPos.Y ) );

			// Now, we need to scale, to match image space
			float scale = Math.Min( (float) m_PreviewBuffer.Width / (float) m_PreviewWidth, (float) m_PreviewBuffer.Height / (float) m_PreviewHeight );

			var imgX = (int) Math.Min( m_PreviewWidth - 1, clientPos.X / scale );
			var imgY = (int) Math.Min( m_PreviewHeight - 1, clientPos.Y / scale );

			// Now, we need to get the color at this point
			long memOffset = ( ( imgY * m_PreviewWidth ) + imgX ) * 4L;
			byte r = m_ImagePreviewData[ memOffset ];
			byte g = m_ImagePreviewData[ memOffset + 1L ];
			byte b = m_ImagePreviewData[ memOffset + 2L ];
			byte a = m_ImagePreviewData[ memOffset + 3L ];

			// Update the cursor label
			cursorLabel.Text = "Cursor: " + imgX + "x" + imgY + " [" + ( int ) r + ", " + ( int ) g + ", " + ( int ) b + ", " + ( int ) a + "]";
		}


		private void CenterImagePanel( bool bResetOffset = true )
		{
			if( bResetOffset )
			{
				// Whenever we center the image panel, cancel the current pan operation, and reset everything related to it
				m_Dragging = false;
				m_DragBegin = Point.Empty;
				m_PreviewOffsetX = 0;
				m_PreviewOffsetY = 0;
			}

			// First, get the size of the parent panel
			var newX = ( previewPanel.ClientSize.Width / 2 ) - ( imagePanel.Width / 2 ) + m_PreviewOffsetX;
			var newY = ( previewPanel.ClientSize.Height / 2 ) - ( imagePanel.Height / 2 ) + m_PreviewOffsetY;

			imagePanel.Location = new Point( newX, newY );

		}


		private float CalculateDefaultZoomFactor( int Width, int Height )
		{
			// We want to aim for 75% of the size of the available panel
			float targetW = previewPanel.ClientSize.Width * 0.75f;
			float targetH = previewPanel.ClientSize.Height * 0.75f;

			// Now, calculate a factor for each, and then we have to decide on a final one, which gets us
			// as close to the target, as possible, without being bigger than th eavailable panel
			float wFactor = targetW / (float) Width;
			float HFactor = targetH / (float) Height;

			// Test 'wFactor'
			float wFactorHeight = (float) Height * wFactor;
			float wFactorArea = wFactorHeight * targetW;

			// Test 'hFactor'
			float hFactorWidth = (float) Width * HFactor;
			float hFactorArea = hFactorWidth * targetH;

			// Test average between the two
			float avgFactor = ( wFactor + HFactor ) / 2.0f;
			float avgFactorW = (float) Width * avgFactor;
			float avgFactorH = (float) Height * avgFactor;
			float avgFactorArea = avgFactorW * avgFactorH;

			// Now, we want to find whici of these, dont overflow, and have the greatest area
			// So, first test for overflow
			bool bW = ( wFactorHeight <= previewPanel.ClientSize.Height ); // wFactor doesnt overflow
			bool bH = ( hFactorWidth <= previewPanel.ClientSize.Width ); // hFactor doesnt overflow
			bool bAvg = ( avgFactorW <= previewPanel.ClientSize.Width && avgFactorH <= previewPanel.ClientSize.Height ); // avgFactor doesnt overflow

			if( bW )
			{
				if( bH )
				{
					if( bAvg )
					{
						// All are true
						return wFactorArea > hFactorArea ? ( wFactorArea > avgFactorArea ? wFactor : ( avgFactorArea > hFactorArea ? avgFactor : HFactor ) ) : ( hFactorArea > avgFactorArea ? HFactor : avgFactor );
					}
					else
					{
						// bH and bW are true
						return hFactorArea > wFactorArea ? HFactor : wFactor;
					}
				}
				else if( bAvg )
				{
					// bH is false, bW and bAvg are true
					return wFactorArea > avgFactorArea ? wFactor : avgFactor;
				}
				else
				{
					// bH and bAvg are false, bW is true
					return wFactor;
				}
			}
			else if( bH )
			{
				// bW = false
				if( bAvg )
				{
					// bW = false, bAvg = true, bH = true
					return avgFactorArea > hFactorArea ? avgFactor : HFactor;
				}
				else
				{
					// bW = false, bAvg = false, bH = true
					return HFactor;
				}
			}
			else if( bAvg )
			{
				// bW = false; bH = false, bAvg = true
				return avgFactor;
			}
			else
			{
				// ERROR Everything overflowed?
				Core.WriteLine( "[ERROR] Failed to calculate default scroll factor for image.. math error??" );
				return 1.0f;
			}
		}


		private void RescalePreviewBuffer( int inW, int inH )
		{
			if( m_PreviewBuffer == null || m_PreviewBuffer.Width != inW || m_PreviewBuffer.Height != inH )
			{
				m_PreviewBuffer = new Bitmap( inW, inH );
			}
		}
		


		private void RenderImageToBuffer()
		{
			// Check if we have everything we need to render
			if( m_PreviewBuffer == null || m_PreviewWidth == 0 || m_PreviewHeight == 0 || ( m_ImagePreviewData?.Length ?? 0 ) < 4 || m_ZoomValue <= 0.0f )
			{
				return;
			}

			// Appears like we have everything, so lets draw
			var bufW = m_PreviewBuffer.Width;
			var bufH = m_PreviewBuffer.Height;

			// Were just going to draw the image to fit to the buffer
			float scale = Math.Min( (float) bufW / (float) m_PreviewWidth, (float) bufH / (float) m_PreviewHeight );

			// TODO: Calculate offsets to ensure, both the rendered image is centered, and we can use these numbers in the mouse
			// label calculation, to get more accurate results

			if( scale == 1.0f )
			{
				// This is a 1:1 drawing of the image, this wont happen often, where the numbers line up exactly
				// But, since we can do this so much more efficiently, we have a special case for it

				using( var gr = Graphics.FromImage( m_PreviewBuffer ) )
				{
					// Loop through each pixel in the output buffer
					for( int y = 0; y < bufH; y++ )
					{
						for( int x = 0; x < bufW; x++ )
						{
							long memOffset = ( ( y * bufW ) + x ) * 4L;

							// Sample the pixel from source at the same location
							byte r = m_ImagePreviewData[ memOffset ];
							byte g = m_ImagePreviewData[ memOffset + 1L ];
							byte b = m_ImagePreviewData[ memOffset + 2L ];
							byte a = m_ImagePreviewData[ memOffset + 3L ];

							// Draw this pixel on the output buffer
							gr.FillRectangle( new SolidBrush( Color.FromArgb( a, r, g, b ) ), x, y, 1, 1 );
						}
					}
				}
			}
			else
			{
				// This is a more complicated algorithm, that can handle both scaling up and down (hopefully!)
				// Basically, it works by looping through each buffer pixel, coming up with a list of image pixels that will contribute to it
				// Then, it loops through each of the contributing pixels, and builds a square over the image, this square represents the area were
				// going to sample from, and it calculate how much this sample square overlaps the current pixel (represented by a 1x1 square)
				// The area of the overlap, is used to weight the pixel sample, against the others contributing to the buffer pixel output
				using( var gr = Graphics.FromImage( m_PreviewBuffer ) )
				{
					// This is the size of a buffer pixel, but in image space
					float bufPixelSize = 1.0f / scale;

					for( int y = 0; y < bufH; y++ )
					{
						for( int x = 0; x < bufW; x++ )
						{
							// Calculate position in 'image space'
							float imgX = x / scale;
							float imgY = y / scale;

							// Now, we need to figure out a list of pixels that contribute to this
							// Since were scaling up, it should only be 1 to 4 pixels
							int imgXBegin = (int) Math.Floor( imgX );
							int imgYBegin = (int) Math.Floor( imgY );
							int imgXEnd = Math.Min( (int) Math.Ceiling( imgX + bufPixelSize ), m_PreviewWidth );
							int imgYEnd = Math.Min( (int) Math.Ceiling( imgY + bufPixelSize ), m_PreviewHeight );

							int accumR = 0, accumG = 0, accumB = 0, accumA = 0;
							float totalArea = 0.0f;

							// Now, know which pixels we want to use, but we also need to be able to determine how much they affect the output pixel
							// This is based on position of the buffer pixel, in image space (and the size)
							for( int iy = imgYBegin; iy < imgYEnd; iy++ )
							{
								for( int ix = imgXBegin; ix < imgXEnd; ix++ )
								{
									// Find memory offset for the image pixel at this position
									long memOffset = ( ( iy * m_PreviewWidth ) + ix ) * 4L;

									// If alpha == 0, totally ignore this pixel
									if( m_ImagePreviewData[ memOffset + 3L ] == (byte) 0 )
									{
										continue;
									}

									// Basically, were representing each pixel with a square, a buffer pixel is smaller than an image pixel
									// We overlay the buffer pixel onto the image, and figure out which pixels were overlapping, and by how much
									// So, were looping through each pixel in the image, that overlaps the buffer pixel
									// Were going to clamp the image pixel square, to the buffer pixel square, and calculate the area
									float overlapXBegin = Math.Max( (float)ix, imgXBegin );
									float overlapXEnd = Math.Min( (float)ix + 1.0f, imgXEnd );
									float overlapYBegin = Math.Max( (float)iy, imgYBegin );
									float overlapYEnd = Math.Min( (float)iy + 1.0f, imgYEnd );

									// Calculate overlap area
									float overlapArea = ( overlapXEnd - overlapXBegin ) * ( overlapYEnd - overlapYBegin );

									// Now calculate how much we are contributing to the pixel, accumulate values
									accumR += ( int ) ( overlapArea * ( float ) m_ImagePreviewData[ memOffset ] );
									accumG += ( int ) ( overlapArea * ( float ) m_ImagePreviewData[ memOffset + 1L ] );
									accumB += ( int ) ( overlapArea * ( float ) m_ImagePreviewData[ memOffset + 2L ] );
									accumA += ( int ) ( overlapArea * ( float ) m_ImagePreviewData[ memOffset + 3L ] );

									totalArea += overlapArea;

								}
							}

							// Now, calculate the final color of this pixel
							byte finalR = (byte) Math.Min( 255.0f, accumR / totalArea );
							byte finalG = (byte) Math.Min( 255.0f, accumG / totalArea );
							byte finalB = (byte) Math.Min( 255.0f, accumB / totalArea );
							byte finalA = (byte) Math.Min( 255.0f, accumA / totalArea );

							// Write this to the output buffer
							gr.FillRectangle(
								new SolidBrush( Color.FromArgb( finalA, finalR, finalG, finalB ) ),
								x, y, 1.0f, 1.0f );
						}
					}
				}
			}
		}


		private void DrawImagePreview( bool bRepaintControl = false )
		{
			// Now, this event needs m_ImagePreviewData, m_PreviewWidth, m_PreviewHeight set 
			if( ( m_ImagePreviewData?.Length ?? 0 ) < 4 || m_PreviewWidth < 1 || m_PreviewHeight < 1 )
			{
				// No image set, clear members and the preview panel
				m_ImagePreviewData	= null;
				m_PreviewWidth		= 0;
				m_PreviewHeight		= 0;

				// Ensure the zoom control is disabled
				m_ZoomControl.Enabled = false;
				m_ZoomControl.SetStepAmount( 0.1f );

				// Rescale the buffer, ensure it cleared with transparency, then resize the image panel
				RescalePreviewBuffer( 1, 1 );

				using( var g = Graphics.FromImage( m_PreviewBuffer ) )
				{
					g.Clear( Color.Transparent );
				}

				imagePanel.Size = new Size( 1, 1 );
			}
			else
			{
				// Now, we need to figure out the default zoom value
				var zoomFactor = CalculateDefaultZoomFactor( m_PreviewWidth, m_PreviewHeight );
				// Clamp the zoom factor between 10% and 100x (i.e. 10000%)
				m_ZoomValue = Math.Max( MinZoomValue, Math.Min( MaxZoomValue, zoomFactor ) );

				if( m_ZoomValue >= 10.0f )
				{
					m_ZoomValue = ( float ) Math.Floor( m_ZoomValue ); // Round to 100% values
					m_ZoomControl.SetStepAmount( 1.0f );
				}
				else if( m_ZoomValue >= 5.0f )
				{
					// Round to nearest 50%
					m_ZoomValue = ( float ) ( Math.Floor( m_ZoomValue * 5.0f ) / 5.0f );
					m_ZoomControl.SetStepAmount( 0.5f );
				}
				else
				{
					m_ZoomValue = ( float ) ( Math.Floor( m_ZoomValue * 10.0f ) / 10.0f ); // Round to 10% values
					m_ZoomControl.SetStepAmount( 0.1f );
				}

				// Silently update the zoom control, so it doesnt cause an event to fire to resize the iamge
				// Also, set the reset value, so when zoom is reset, it goes back to this default value
				m_ZoomControl.SilentSetZoom( m_ZoomValue );
				m_ZoomControl.SetResetValue( m_ZoomValue );

				// Now, lets calculate the size of the panel we need to draw this image on, using the zoom factor
				int finalW = (int) Math.Ceiling( m_PreviewWidth * m_ZoomValue );
				int finalH = (int) Math.Ceiling( m_PreviewHeight * m_ZoomValue );

				// We need to rescale the buffer, if were not re-rendering, then keep current contents
				RescalePreviewBuffer( finalW, finalH );

				// Draw on the buffer 
				RenderImageToBuffer();

				// Resize the image
				imagePanel.Size = new Size( finalW, finalH );

				// Enable the zoom control
				m_ZoomControl.Enabled = true;
			}

			// Ensure the image panel is centered on the preview panel
			CenterImagePanel();

			// Invalidate the preview panel, to force it to redraw
			if( bRepaintControl )
			{
				// TODO: See if this is actually needed, since were now resizing the panel directly int his function
				imagePanel.Invalidate();
			}
		}


		private void OnZoomChanged( object sender, float newValue )
		{
			// Check if we should switch the zoom type
			if( newValue >= 10.0f )
			{
				m_ZoomControl.SetStepAmount( 1.0f );
			}
			else if( newValue >= 5.0f )
			{
				m_ZoomControl.SetStepAmount( 0.5f );
			}
			else
			{
				m_ZoomControl.SetStepAmount( 0.1f );
			}

			if( m_PreviewBuffer != null && m_PreviewWidth != 0 && m_PreviewHeight != 0 )
			{
				// Disable the zoom button until the next re-draw
				m_ZoomControl.Enabled = false;

				// Calculate the new buffer size based on the zoom factor
				m_ZoomValue = newValue;

				int newW = (int) Math.Ceiling( m_PreviewWidth * newValue );
				int newH = (int) Math.Ceiling( m_PreviewHeight * newValue );

				// Resize the image panel
				imagePanel.Size = new Size( newW, newH );

				CenterImagePanel( false );
				imagePanel.Invalidate();
			}
		}


		private void lodList_SelectedIndexChanged( object sender, EventArgs e )
		{
			// When the selected LOD changes, we need to update a lot, we have to switch the active image
			// The way we want to do this, is whenever an LOD is selected, we decode it into raw image data
			// and store the buffer as a member, and then render off from that each repaint
			m_ImagePreviewData = null;

			int newIndex = lodList.SelectedIndex;
			if( newIndex >= 0 || ( m_CurrentTexture?.LODs?.Length ?? 0 ) > newIndex )
			{ 
				// Valid index selected, we need to get the data, decode it, and store it
				var sourceData = m_CurrentTexture.LODs[ newIndex ].Data;
				if( ( sourceData?.Length ?? 0 ) > 0 )
				{
					byte[] rawData = null;

					switch( m_CurrentTexture.Format )
					{
						case TextureFormat.RGBA_8BIT_UNORM:
							rawData = sourceData; // We wont copy it, since were not going to modify it at all
							break;
						case TextureFormat.RGBA_8BIT_UNORM_SRGB:
							// TODO
							throw new NotImplementedException( "SRGB Previews are not enabled" ); // TODO TODO
						case TextureFormat.RGBA_BC_7:
						case TextureFormat.RGB_BC_7:

							var bc7_res = BC7Encoder.Decode( sourceData, m_CurrentTexture.LODs[ newIndex ].Width, m_CurrentTexture.LODs[ newIndex ].Height, m_CurrentTexture.LODs[ newIndex ].RowSize,
								m_CurrentTexture.Format == TextureFormat.RGB_BC_7, out rawData );

							if( !bc7_res )
							{
								Core.WriteLine( "[Warning] TextureViewer: Failed to read BC7 texture, decoding failed!" );
								rawData = null;
							}

							break;

						case TextureFormat.RGBA_DXT_5:

							var dxt5_res = DXT5Encoder.Decode( sourceData, m_CurrentTexture.LODs[ newIndex ].Width, m_CurrentTexture.LODs[ newIndex ].Height,
								m_CurrentTexture.LODs[ newIndex ].RowSize, out rawData );

							if( !dxt5_res )
							{
								Core.WriteLine( "[Warning] TextureViewer: Failed to read DXT5 texture, decoding failed!" );
								rawData = null;
							}

							break;

						case TextureFormat.RGB_DXT_1:

							var dxt1_res = DXT1Encoder.Decode( sourceData, m_CurrentTexture.LODs[ newIndex ].Width, m_CurrentTexture.LODs[ newIndex ].Height, 
								m_CurrentTexture.LODs[ newIndex ].RowSize, out rawData );

							if( !dxt1_res )
							{
								Core.WriteLine( "[Warning] TextureViewer: Failed to read DXT1 texture, decoding failed!" );
								rawData = null;
							}

							break;

						case TextureFormat.RG_8BIT_UNORM:
						case TextureFormat.R_8BIT_UNORM:

							var gsa_res = GrayscaleEncoder.Decode( sourceData, m_CurrentTexture.LODs[ newIndex ].Width, m_CurrentTexture.LODs[ newIndex ].Height,
								m_CurrentTexture.Format == TextureFormat.RG_8BIT_UNORM, out rawData );

							if( !gsa_res )
							{
								Core.WriteLine( "[Warning] TextureViewer: Failed to read GS texture, decoding failed!" );
								rawData = null;
							}

							break;

						default:
							throw new InvalidDataException( "Current texture has an invalid format?" );
					}

					if( rawData != null && rawData.Length >= 4 )
					{
						// Set the preview data
						m_ImagePreviewData	= rawData;

						m_PreviewWidth		= (int)m_CurrentTexture.LODs[ newIndex ].Width;
						m_PreviewHeight		= (int)m_CurrentTexture.LODs[ newIndex ].Height;
					}
				}

				// Update the labels
				var lod = m_CurrentTexture.LODs[ newIndex ];
				lodSizeLabel.Text = "LOD Size: " + lod.Width + "x" + lod.Height + " \n" + SizeInBytesToString( lod.Data?.LongLength ?? 0L );
			}
			else
			{
				// No valid LOD selected
				lodSizeLabel.Text = "LOD Size: N/A \n0 bytes [BAD INDEX]";
			}

			// Call Events
			DrawImagePreview( true );
			UpdateCursorLabel();
		}

		private void browseButton_Click( object sender, EventArgs e )
		{
			// We need to open a file browser, to select an .htx file
			using( var fSelect = new OpenFileDialog()
			{
				AddExtension = false,
				CheckFileExists = true,
				CheckPathExists = true,
				Multiselect = false,
				InitialDirectory = Directory.GetCurrentDirectory(),
				ValidateNames = true,
				DereferenceLinks = true,
				ShowReadOnly = false,
				Title = "Select texture file...",
				SupportMultiDottedExtensions = false,
				ShowHelp = false,
				Filter = "texture files (*.htx)|*.htx",
				FilterIndex = 0
			} )
			{
				if( fSelect.ShowDialog( this ) == DialogResult.OK )
				{
					// File was selected!
					if( OpenTexture( fSelect.FileName ) )
					{
						Core.WriteLine( "TextureViewer: File opened! \"" + fSelect.FileName + "\"" );
					}
					else
					{
						Core.WriteLine( "TextureViewer: Failed to open \"" + fSelect.FileName + "\"" );
					}
				}
			}
		}

		private void clearButton_Click( object sender, EventArgs e )
		{
			ResetUI();
		}

		private void imagePanel_Paint( object sender, PaintEventArgs e )
		{
			// Draw the preview buffer onto the panel, it should already match the size of the panel, since whenever the panel
			// since we control the size of the panel
			if( m_PreviewBuffer != null )
			{
				e.Graphics.DrawImage( m_PreviewBuffer, 0, 0, imagePanel.Width, imagePanel.Height );
			}
		
			// Ensure zoom buttons are enabled
			if( m_ZoomControl != null )
			{
				m_ZoomControl.Enabled = true;
			}
		}

		private void imagePanel_MouseMove( object sender, MouseEventArgs e )
		{
			// Update the cursor label
			UpdateCursorLabel();

			// Check if were panning
			if( m_Dragging && m_ImagePreviewData != null && m_PreviewBuffer != null )
			{
				// Calculate offset
				var offset = new Point(
					MousePosition.X - m_DragBegin.X,
					MousePosition.Y - m_DragBegin.Y );

				// Apply offset
				m_PreviewOffsetX += offset.X;
				m_PreviewOffsetY += offset.Y;

				// Update image position
				CenterImagePanel( false );

				// Set 'DragBegin' to the mouse position, so we can read how much it moved since
				// last call, when this runs again
				m_DragBegin = MousePosition;
			}
		}

		private void TextureViewer_Resize( object sender, EventArgs e )
		{
			// TODO
			// Need to write code to ensure everything resizes properly

			// Ensure the zoom control is in the proper location
			PositionZoomControl();
		}

		private void imagePanel_MouseDown( object sender, MouseEventArgs e )
		{
			if( !m_Dragging )
			{
				m_Dragging = true;
				m_DragBegin = MousePosition;
			}
		}

		private void imagePanel_MouseUp( object sender, MouseEventArgs e )
		{
			if( m_Dragging )
			{
				m_Dragging = false;
			}
		}

		private void onScroll( object sender, ScrollEventArgs e )
		{
			Core.WriteLine( "[DEBUG] OnScroll!" );
		}
	}
}
