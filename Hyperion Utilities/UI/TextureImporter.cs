using System;
using System.Collections.Generic;
using System.Windows.Forms;
using System.IO;
using System.Drawing;

namespace Hyperion
{
	public struct TextureLODParameters
	{
		public UI.ColorChangeParameters colorParams;
		public byte alphaParam;
	}


	public partial class TextureImporter : Form
	{
		public uint lodCount;
		public TextureImportState state;

		public TextureImporter()
		{
			state.Format = TextureFormat.NONE;
			state.Path = null;
			state.LODs = new List<TextureImportLOD>();

			state.LODs.Add( new TextureImportLOD() );

			lodCount = 1;
			InitializeComponent();
			lodList.Items.Add( 0 );

			// Were going to clear out any LOD editor controls
			clearLODButton.Enabled = false;
			browseButton.Enabled = false;
			autogenButton.Enabled = false;

			// This will force the creation of the buffer, so we can start drawing image previews
			panel7_Resize( this, null );
		}

		private string ConvertColorParams( UI.ColorChangeParameters inVal )
		{
			string outVal = "";
			if( inVal.r )
			{
				outVal += "R";
			}

			if( inVal.g )
			{
				outVal += "G";
			}

			if( inVal.b )
			{
				outVal += "B";
			}

			return outVal;
		}

		private void importButton_Click( object sender, EventArgs e )
		{
			// We need to gather all of the data together and generte a texture file
			// First, lets chcek if everything seems good before going ahead
			if( state.LODs.Count <= 0 )
			{
				Core.WriteLine( "[Warning] TextureImporter: Failed to import.. there are no LODs!" );
				MessageBox.Show( "There are no valid LODs!", "Failed to import!", MessageBoxButtons.OK, MessageBoxIcon.Warning );

				return;
			}

			// Now, we want to validate the actual data for each LOD level
			uint lastWidth		= state.LODs[ 0 ].Width;
			uint lastHeight		= state.LODs[ 0 ].Height;
			
			/*
			for( int i = 1; i < state.LODs.Count; i++ )
			{
				if( state.LODs[ i ].Width != ( lastWidth / 2 ) ||
					state.LODs[ i ].Height != ( lastHeight / 2 ) )
				{
					Core.WriteLine( "[Warning] TextureImporter: Failed to import.. the LOD resolutions are invalid.. each needs to be half of the previous level." );
					Core.WriteLine( "\tLevel ", ( i - 1 ), " is ", lastWidth, "px X ", lastHeight, "px and level ", i, " is ", state.LODs[ i ].Width, "px X ", state.LODs[ i ].Height, "px" );
					MessageBox.Show( "The LOD resolutions are invalid.. refer to console for more details", "Failed to import!", MessageBoxButtons.OK, MessageBoxIcon.Warning );

					return;
				}
			}
			*/ // TODO: Uncomment this, just for testing purposes

			// Validate the path
			if( ( pathBox.Text?.Length ?? 0 ) < 5 || !pathBox.Text.EndsWith( ".htx" ) )
			{
				Core.WriteLine( "[Warning] TextureImporter: failed to import.. invalid output path (must include '.htx' extension)" );
				MessageBox.Show( "Invalid output path! Must also include '.htx' extension", "Failed to import", MessageBoxButtons.OK, MessageBoxIcon.Warning );

				return;
			}

			// Check if there is a file already in the target location
			string fullPath;
			string relPath;
			if( pathBox.Text.StartsWith( "content/" ) )
			{
				fullPath = pathBox.Text;
				relPath = pathBox.Text.Substring( 8 );
			}
			else
			{
				fullPath = "content/" + pathBox.Text;
				relPath = pathBox.Text;
			}

			bool bNewFile = true;
			if( File.Exists( fullPath ) )
			{
				var res = MessageBox.Show( "Do you want to overwrite \"" + fullPath + "\"?", "Texture already exists...", MessageBoxButtons.YesNo, MessageBoxIcon.Question, MessageBoxDefaultButton.Button2 );
				if( res == DialogResult.No )
				{
					Core.WriteLine( "[Warning] TextureImporter: Import canceled..." );
					return;
				}
				else
				{
					bNewFile = false;
				}
			}

			// Calculate the asset hash
			uint assetHash = bNewFile ? Core.GetManifestManager().CalculateAssetId( relPath ) : 0;

			// Get the texture format as a proper enum
			var formatIndex = formatBox.SelectedIndex;
			TextureFormat format = TextureFormat.NONE;

			switch( formatIndex )
			{
				case 0:
					format = TextureFormat.R_8BIT_UNORM;
					break;
				case 1:
					format = TextureFormat.RG_8BIT_UNORM;
					break;
				case 2:
					format = TextureFormat.RGBA_8BIT_UNORM;
					break;
				case 3:
					format = TextureFormat.RGBA_8BIT_UNORM_SRGB;
					break;
				case 4:
					format = TextureFormat.RGB_DXT_1;
					break;
				case 5:
					format = TextureFormat.RGBA_DXT_5;
					break;
				case 6:
					format = TextureFormat.RGB_BC_7;
					break;
				case 7:
					format = TextureFormat.RGBA_BC_7;
					break;
				default:
					Core.WriteLine( "[ERROR] TextureImporter: Invalid format selected?? Import failed" );
					MessageBox.Show( "Invalid output format selected", "Failed to import!", MessageBoxButtons.OK, MessageBoxIcon.Warning );
					return;
			}

			// We need to set these so the import options menu can display them
			state.Format	= format;
			state.Path		= relPath;

			// We need to get some more info from the user, about indivusal LOD levels
			var importOptions = new TextureImportOptions( this );
			var result = importOptions.ShowDialog();
			var lodParams = new TextureLODParameters[ state.LODs.Count ];

			if( result == DialogResult.OK )
			{
				// We got everything we need
				Core.WriteLine( "Texture Importer: Begining texture import for \"", relPath, "\"...." );

				// Copy in the parameters
				for( int i = 0; i < importOptions.m_ImportSettings.Length; i++ )
				{
					lodParams[ i ].colorParams = importOptions.m_ImportSettings[ i ].m_ColorParam;
					lodParams[ i ].alphaParam = importOptions.m_ImportSettings[ i ].m_AlphaParam;

					Core.WriteLine( "[DEBUG] TextureImporter: LOD Params [", i.ToString(), "]  Color Params: " + ConvertColorParams( lodParams[ i ].colorParams ) + "\tAlpha Param: " + lodParams[ i ].alphaParam.ToString() );
				}
			}
			else if( result == DialogResult.Cancel )
			{
				// User clicked cancel button
				Core.WriteLine( "Texture Importer: User canceled texture import..." );
				return;
			}
			else
			{
				// Error or other issue!
				Core.WriteLine( "[Warning] TextureImporter: Failed to select import options for texture!" );
				MessageBox.Show( "Failed to select parameters for LODs?", "Texture import error!", MessageBoxButtons.OK, MessageBoxIcon.Warning );
				return;
			}

			// Now, we have everything we need to complete the import
			// Start creating texture object to be written
			var tex = new Texture( relPath, assetHash, format, 0, state.LODs.Count );

			if( !BuildTexture( tex, lodParams ) )
			{
				MessageBox.Show( "Failed to build texture! Check console for more info", "Texture import error!", MessageBoxButtons.OK, MessageBoxIcon.Warning );
				return;
			}

			// Now, we have to actually write out the texture
			if( !TextureManager.SaveTexture( tex ) )
			{
				MessageBox.Show( "Failed to save texture! Check console for more info", "Texture import error!", MessageBoxButtons.OK, MessageBoxIcon.Warning );
				return;
			}

			// Write should have been successful
			Core.WriteLine( "Texture Importer => Texture successfully imported! \"", relPath, "\" has been saved to disk" );
			Close();
		}

		private void resetButton_Click( object sender, EventArgs e )
		{
			state.Format = TextureFormat.NONE;
			state.Path = null;
			state.LODs.Clear();
			state.LODs.Add( new TextureImportLOD() );

			// Reset everything
			lodList.ClearSelected();
			lodList.Items.Clear();
			lodList.Items.Add( 0 );

			pathBox.Clear();
			formatBox.ResetText();

			lodCount = 1;

			// Select 0 index
			lodList.SelectedIndex = 0;
		}

		private void cancelButton_Click( object sender, EventArgs e )
		{
			Close();
		}

		private void addLODButton_Click( object sender, EventArgs e )
		{
			if( lodCount == 15 )
			{
				Core.WriteLine( "[Warning] TextureImporter: Cannot add another LOD, max number of LODs for a texture is 15" );
				return;
			}

			// Add a new LOD to the list
			uint newIndex = lodCount++;

			if( !lodList.Items.Contains( newIndex ) )
			{
				lodList.Items.Add( newIndex );
			}

			for( int i = state.LODs.Count; i < lodCount; i++ )
			{
				state.LODs.Add( new TextureImportLOD() );
			}

			lodList.SelectedItem = newIndex;
		}

		private void removeLODButton_Click( object sender, EventArgs e )
		{
			if( lodCount <= 1 )
			{
				Core.WriteLine( "[Warning] TextureImporter: Cannot remove LOD, needs at least one!" );
				return;
			}

			// Remove the lowest index from the list
			uint removeIndex = --lodCount;

			if( lodList.Items.Count > removeIndex )
			{
				// If this is the currently selected index, then select the next lowest one (if available)
				if( ( uint ) lodList.SelectedIndex == removeIndex )
				{
					lodList.SelectedIndex = ( int ) removeIndex - 1;
				}

				lodList.Items.Remove( removeIndex );
			}

			for( int i = ( int ) lodCount; i < state.LODs.Count; i++ )
			{
				state.LODs.RemoveAt( i );
			}
		}

		private void ClearLODEditor()
		{
			selectedLODLabel.Text = "Selected LOD: <none>";
			sizeLabel.Text = "LOD Size: 0kb";
			resolutionLabel.Text = "0px X 0px";
			totalSizeLabel.Text = "Total Raw Size: 0kb";
			imagePathBox.Clear();
			autogenButton.Enabled = false;
			browseButton.Enabled = false;
			clearLODButton.Enabled = false;
			//previewBox.Image = null;
			DrawImagePreview();
		}

		private void UpdateLabels( int lodIndex )
		{
			// The total size label doesnt depend on the current index
			long totalSize = 0;
			foreach( var level in state.LODs )
			{
				totalSize += ( level.Data?.Length ?? 0 );
			}

			totalSizeLabel.Text = ( "Total Raw Size: " + ( totalSize / 1024L ).ToString() + "kb" );

			if( lodIndex < 0 || lodIndex >= state.LODs.Count )
			{
				Core.WriteLine( "[Warning] TextureImporter: Failed to update labels.. invalid index? (", lodIndex, ")" );

				selectedLODLabel.Text = "Selected LOD: <none>";
				sizeLabel.Text = "LOD Size: 0kb";
				resolutionLabel.Text = "0px X 0px";
		
				return;
			}

			selectedLODLabel.Text = ( "Selected LOD: " + lodIndex.ToString() );
			sizeLabel.Text = ( "LOD Size: " + ( ( state.LODs[ lodIndex ].Data?.Length ?? 0 ) / 1024L ).ToString() + "kb" );

			if( state.LODs[ lodIndex ].SourceFormat == ImageFormat.NONE )
			{
				resolutionLabel.Text = "0px X 0px";
			}
			else
			{
				resolutionLabel.Text = ( state.LODs[ lodIndex ].Width.ToString() + "px X " + state.LODs[ lodIndex ].Height.ToString() + "px" ) +
					"[" + Enum.GetName( typeof( ImageFormat ), state.LODs[ lodIndex ].SourceFormat ) + "]";
			}
		}

		private void UpdatePreview( int lodIndex )
		{
			// We need to load the source image into the box, were not going to bother using the converted version being held in memory
			// especially since we would need to run another conversion, and it would be a total pain in the ass
			if( lodIndex < 0 || lodIndex >= state.LODs.Count || ( state.LODs[ lodIndex ].ImagePath?.Length ?? 0 ) <= 4 )
			{
				//previewBox.Image = null;
			}
			else
			{
				//previewBox.Image = System.Drawing.Image.FromFile( state.LODs[ lodIndex ].ImagePath );
			}
		}

		private void lodList_SelectedIndexChanged( object sender, EventArgs e )
		{
			if( lodList.SelectedItem == null )
			{
				// No LODs are selected
				ClearLODEditor();
			}
			else
			{
				// Get the current value
				var selectedIndex = lodList.SelectedIndex;

				if( selectedIndex >= state.LODs.Count || state.LODs[ selectedIndex ] == null )
				{
					Core.WriteLine( "[Warning] TextureImporter: Failed to select LOD level (", selectedIndex, ") because we dont have data for this level" );
					ClearLODEditor();
					return;
				}

				UpdateLabels( selectedIndex );
				//UpdatePreview( selectedIndex );
				DrawImagePreview();

				if( state.LODs.Count <= selectedIndex )
				{
					autogenButton.Text = "Export";
					autogenButton.Enabled = false;
					browseButton.Enabled = false;
					clearLODButton.Enabled = false;
					imagePathBox.Text = "";
				}
				else if( state.LODs[ selectedIndex ].Autogenerated )
				{
					state.LODs[ selectedIndex ].ImagePath = null;

					if( selectedIndex == 0 )
					{
						Core.WriteLine( "[ERROR] TextureImport: LOD index 0 is autogenerated?? This should not happen!" );
						Close();
					}
					else if( ( state.LODs[ selectedIndex ].Data?.Length ?? 0 ) == 0 )
					{
						Core.WriteLine( "[Warning] TextureImport: LOD Index ", selectedIndex, " was autogenerated, but there is no data" );

						state.LODs[ selectedIndex ].Autogenerated = false;

						autogenButton.Enabled = false;
						autogenButton.Text = "Export";
						browseButton.Enabled = true;
						clearLODButton.Enabled = true;
						imagePathBox.Text = "";
						imagePathBox.Enabled = true;
					}
					else
					{
						autogenButton.Enabled = true;
						autogenButton.Text = "Export";
						browseButton.Enabled = false;
						clearLODButton.Enabled = true;

						imagePathBox.Text = "";
						imagePathBox.Enabled = false;
					}
				}
				else
				{
					if( selectedIndex == 0 )
					{
						autogenButton.Enabled = ( state.LODs[ selectedIndex ].Data?.Length ?? 0 ) > 0;
						autogenButton.Text = "Generate MIPs";
					}
					else
					{
						autogenButton.Enabled = false;
						autogenButton.Text = "Export";
					}

					browseButton.Enabled = true;
					clearLODButton.Enabled = true;

					if( state.LODs[ selectedIndex ].ImagePath != null )
					{
						imagePathBox.Text = state.LODs[ selectedIndex ].ImagePath;
						imagePathBox.Enabled = false;
					}
					else
					{
						imagePathBox.Text = "";
						imagePathBox.Enabled = true;
					}

				}
			}
		}

		private void browseButton_Click( object sender, EventArgs e )
		{
			// Ensure a valid index is selected
			var selectedIndex = lodList.SelectedIndex;
			if( selectedIndex >= state.LODs.Count )
			{
				Core.WriteLine( "[Warning] TextureImporter: Failed to browse for image to use for LOD ", selectedIndex, " because this isnt a valid index" );
				return;
			}
			else if( ( state.LODs[ selectedIndex ].Data?.Length ?? 0 ) > 0 || state.LODs[ selectedIndex ].Autogenerated )
			{
				Core.WriteLine( "[Warning] TextureImporter: Failed to browse for image to use for LOD ", selectedIndex, " because there is already data for this LOD" );
				return;
			}

			// Open image selector
			using( var imageSelector = new OpenFileDialog()
			{
				AddExtension = false,
				CheckFileExists = true,
				CheckPathExists = true,
				Multiselect = false,
				InitialDirectory = Directory.GetCurrentDirectory(),
				ValidateNames = true,
				DereferenceLinks = true,
				ShowReadOnly = false,
				Title = "Select image file...",
				SupportMultiDottedExtensions = false,
				ShowHelp = false,
				Filter = "png files (*.png)|*.png|tga files (*.tga)|*.tga|jpg files (*.jpg)|*.jpg|Image files (*.png;*.tga;*.jpg)|*.png;*.tga;*.jpg",
				FilterIndex = 4
			} )
			{
				if( imageSelector.ShowDialog( this ) == DialogResult.OK )
				{
					var fileName = imageSelector.FileName;

					// Do a quick validation of the path
					if( ( fileName?.Length ?? 0 ) == 0 || !File.Exists( fileName ) )
					{
						Core.WriteLine( "[Warning] ImageSelector: No image was selected!" );
						return;
					}
					else
					{
						var ext = Path.GetExtension( fileName ).ToLower();
						if( ext != ".png" && ext != ".tga" && ext != ".jpg" )
						{
							Core.WriteLine( "[Warning] ImageSelector: Selected invalid image! ", fileName );
							return;
						}
					}

					Core.WriteLine( "[DEBUG] ImageSelector: Selected image to upload: ", fileName );

					// Perform the import of the selected image, this function automatically detects the type and performs the import
					// We have a standardized format we convert to, which is basically just a uncompressed 32-bit per pixel image
					if( !GenericImporter.AutoImport( fileName, out RawImageData imgData ) )
					{
						Core.WriteLine( "[Warning] TextureImporter: Failed to import image for LOD ", selectedIndex, " (", fileName, ") check previous messages for more info" );
						return;
					}
					else
					{
						// Store the image in the selected LOD
						state.LODs[ selectedIndex ].ImagePath = fileName;
						state.LODs[ selectedIndex ].Data = imgData.Data;
						state.LODs[ selectedIndex ].Width = imgData.Width;
						state.LODs[ selectedIndex ].Height = imgData.Height;
						state.LODs[ selectedIndex ].SourceFormat = imgData.Format;

						Core.WriteLine( "[Status] TextureImporter: Imported new image! (", fileName, ") [", imgData.Width, "x", imgData.Height, "] ", Enum.GetName( typeof( ImageFormat ), imgData.Format ) );

						// Update the UI
						imagePathBox.Text = fileName;
						imagePathBox.Enabled = false;
						clearLODButton.Enabled = true;
						browseButton.Enabled = true;
						autogenButton.Enabled = ( selectedIndex == 0 );
						autogenButton.Text = ( selectedIndex == 0 ) ? "Generate MIPs" : "Export";

						UpdateLabels( selectedIndex );
						//UpdatePreview( selectedIndex );
						DrawImagePreview();
					}
				}
			}
		}

		private void TextureImport_FormClosing( object sender, FormClosingEventArgs e )
		{
			Core.WriteLine( "Closing texture importer..." );
		}

		private void TextureImport_Load( object sender, EventArgs e )
		{
			Core.WriteLine( "Opening texture importer..." );
		}

		private void autogenButton_Click( object sender, EventArgs e )
		{
			// Check if were exporting or if were autogenerating mips
			var selectedIndex = lodList.SelectedIndex;

			if( selectedIndex == 0 )
			{
				var res = MessageBox.Show( this, "Autogenerating MIPs will cause any existing LOD levels to be cleared, and replaced. Are you sure?",
					"Confirm Autogeneration", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning );
				if( res == DialogResult.OK )
				{
					AutogenerateMIPs();
				}
			}
			else
			{
				if( ( state.LODs[ selectedIndex ]?.Data?.Length ?? 0 ) > 0 )
				{
					ExportLOD();
				}
			}
		}

		private void clearLODButton_Click( object sender, EventArgs e )
		{
			var selectedIndex = lodList.SelectedIndex;

			if( state.LODs.Count >= selectedIndex )
			{
				if( state.LODs[ selectedIndex ] == null )
				{
					state.LODs[ selectedIndex ] = new TextureImportLOD();
				}

				state.LODs[ selectedIndex ].Autogenerated = false;
				state.LODs[ selectedIndex ].Data = null;
				state.LODs[ selectedIndex ].Width = 0;
				state.LODs[ selectedIndex ].Height = 0;
				state.LODs[ selectedIndex ].ImagePath = null;
				state.LODs[ selectedIndex ].RowSize = 0;
				state.LODs[ selectedIndex ].SourceFormat = ImageFormat.NONE;
			}

			if( selectedIndex == 0 )
			{
				autogenButton.Text = "Generate MIPs";
			}
			else
			{
				autogenButton.Text = "Export";
			}

			autogenButton.Enabled = false;
			browseButton.Enabled = true;
			clearLODButton.Enabled = false;
			imagePathBox.Text = "";
			imagePathBox.Enabled = true;
			//previewBox.Image = null;
			DrawImagePreview();
		}


		private void AutogenerateMIPs()
		{

		}


		private void ExportLOD()
		{

		}


		private bool BuildTexture( Texture inTex, TextureLODParameters[] inParams )
		{
			// Validate the parameters
			if( inTex == null || inTex.LODs.Length != inParams.Length || state.LODs.Count != inTex.LODs.Length )
			{
				Core.WriteLine( "[ERROR] TextureImporter: Failed to import texture... parameter list length didnt match lod count??" );
				return false;
			}

			// We need to go through each LOD level
			for( int i = 0; i < inTex.LODs.Length; i++ )
			{
				// We need to build this LOD to the output format
				bool bResult = false;

				inTex.LODs[ i ].Width = state.LODs[ i ].Width;
				inTex.LODs[ i ].Height = state.LODs[ i ].Height;

				switch( inTex.Format )
				{
					case TextureFormat.RGBA_8BIT_UNORM:
						bResult = RGBEncoder.Encode( state.LODs[ i ], false, inParams[ i ], out inTex.LODs[ i ].Data, out inTex.LODs[ i ].RowSize );
						break;
					case TextureFormat.RGBA_8BIT_UNORM_SRGB:
						bResult = RGBEncoder.Encode( state.LODs[ i ], true, inParams[ i ], out inTex.LODs[ i ].Data, out inTex.LODs[ i ].RowSize );
						break;
					case TextureFormat.RGBA_BC_7:
						bResult = BC7Encoder.Encode( state.LODs[ i ], false, inParams[ i ], out inTex.LODs[ i ].Data, out inTex.LODs[ i ].RowSize );
						break;
					case TextureFormat.RGBA_DXT_5:
						bResult = DXT5Encoder.Encode( state.LODs[ i ], inParams[ i ], out inTex.LODs[ i ].Data, out inTex.LODs[ i ].RowSize );
						break;
					case TextureFormat.RGB_BC_7:
						bResult = BC7Encoder.Encode( state.LODs[ i ], false, inParams[ i ], out inTex.LODs[ i ].Data, out inTex.LODs[ i ].RowSize );
						break;
					case TextureFormat.RGB_DXT_1:
						bResult = DXT1Encoder.Encode( state.LODs[ i ], inParams[ i ], out inTex.LODs[ i ].Data, out inTex.LODs[ i ].RowSize );
						break;
					case TextureFormat.RG_8BIT_UNORM:
						bResult = GrayscaleEncoder.Encode( state.LODs[ i ], true, inParams[ i ], out inTex.LODs[ i ].Data, out inTex.LODs[ i ].RowSize );
						break;
					case TextureFormat.R_8BIT_UNORM:
						bResult = GrayscaleEncoder.Encode( state.LODs[ i ], false, inParams[ i ], out inTex.LODs[ i ].Data, out inTex.LODs[ i ].RowSize );
						break;
					default:
						Core.WriteLine( "[ERROR] TextureImporter: Invalid format type..." );
						return false;
				}

				if( !bResult )
				{
					Core.WriteLine( "Failed to encode LOD " + i.ToString() );
					return false;
				}
			}

			return true;
		}

		/*-----------------------------------------------------------------------------------------
		 *	Preview Drawing
		------------------------------------------------------------------------------------------*/
		private Bitmap m_PreviewBuffer;

		private void panel7_Paint( object sender, PaintEventArgs e )
		{
			// We want to test painting the image ourself, instead of using a picture box
			// This would allow us to paint images stored directly in memory, instead of having to use the file, or
			// processing an image in memory as another format before displaying, this will be much more efficient in theory
			e.Graphics.DrawImageUnscaled( m_PreviewBuffer, Point.Empty );
		}

		private void panel7_Resize( object sender, EventArgs e )
		{
			if( m_PreviewBuffer == null || m_PreviewBuffer.Width < panel7.Width || m_PreviewBuffer.Height < panel7.Height )
			{
				Bitmap newBuffer = new Bitmap( panel7.Width, panel7.Height );

				if( m_PreviewBuffer != null )
				{
					using( Graphics g = Graphics.FromImage( newBuffer ) )
					{
						g.DrawImageUnscaled( m_PreviewBuffer, Point.Empty );
					}
				}

				m_PreviewBuffer = newBuffer;
			}
		}

		private void DrawImagePreview()
		{
			if( m_PreviewBuffer != null )
			{
				using( Graphics g = Graphics.FromImage( m_PreviewBuffer ) )
				{
					// Each image is just a bunch of pixels, the issue is, we need to figure out how many real pixels are needed to draw a single source pixel
					// We want to get an even number, and could potentially change the size of the image on screen to accomidate
					// So.. if we had an image thats 20 x 20, and we have 30 x 30 pixels to draw it with...
					// 30 / 20 = 1.5
					// But, since we dont want to deal in half pixels, we would instead use 1, and the image on screen would end up taking a little less space
					// This is okay, we want to ensure we stay within our bounds though

					var cnvW = m_PreviewBuffer.Width;
					var cnvH = m_PreviewBuffer.Height;

					var selectedIndex = lodList.SelectedIndex;
					if( state.LODs.Count <= selectedIndex || ( ( state.LODs[ selectedIndex ].Data?.Length ?? 0 ) == 0 ) )
					{
						// Invalid LOD selected, clear everything
						g.Clear( panel7.BackColor );
					}
					else
					{
						var sourceData = state.LODs[ selectedIndex ].Data;

						var imgW = state.LODs[ selectedIndex ].Width;
						var imgH = state.LODs[ selectedIndex ].Height;

						// Now, determine how many canvas pixels will be used to cover a pixel from the image
						float wratio = ( float ) cnvW / ( float ) imgW;
						float hratio = ( float ) cnvH / ( float ) imgH;

						// Now, say we have a 30 x 30 canvas
						// And a 10 x 20 image
						// wratio = 3
						// hratio = 1.5
						//
						// We want to select the lowest ratio, to ensure the image always fits
						// If we applied wratio to this, the required hieght in canvas pixels would be 60, instead of 30
						float pixelRatio = Math.Min( wratio, hratio );

						// Now if the ratio is >= 1, this is fairly easy, we will round it down to the nearest whole value, and draw it
						// If the ratio is < 1, this means we need to do some interpolating in our drawing loop, and it gets more complicated
						// We need to come up with a number of pixels from the image, to represent in a single pixel on our screen
						if( pixelRatio >= 1.0f )
						{
							var finalPixelRatio = (int)Math.Floor( pixelRatio );

							// Now, the drawing of the image will be quite easy
							// Loop through each pixel in source image
							for( uint y = 0; y < imgH; y++ )
							{
								for( uint x = 0; x < imgW; x++ )
								{
									long memOffset = ( ( y * imgW ) + x ) * 4L;

									g.FillRectangle( 
										new SolidBrush( Color.FromArgb( 
											sourceData[ memOffset + 3L ], 
											sourceData[ memOffset ], 
											sourceData[ memOffset + 1L ], 
											sourceData[ memOffset + 2L ] ) ), 
										new Rectangle( (int)x * finalPixelRatio, (int)y * finalPixelRatio, finalPixelRatio, finalPixelRatio ) 
										);
								}
							}
						}
						else
						{
							// Here, we still have a lot of work to do, we need to split the image up into blocks, and draw those blocks
							// We need to determine block size, based on the canvas resolution and the source image resolution
							//
							// Say the image is 100 x 100
							// Canvas is 30 x 30
							// Our ratios are 0.3 (for all of them)
							// Now, to make this work, we would need to use 4x4 grids at least
							// How to get to this number? Well, we need a width and height number
							//
							// 100 / 30 => 3.333 => Ceil => 4
							int blockW = (int) Math.Ceiling( (float) imgW / (float) cnvW );
							int blockH = (int) Math.Ceiling( (float) imgH / (float) cnvH );
							int blockSize = Math.Max( blockW, blockH );

							// Now, we need to figure out how many blocks are in the source image..
							// So, in previous example.. the image was 100 x 100, panel is 30 x 30
							// Block size ended up being 4
							// So.. there is going to be 100 / 4 blocks
							var blockWCount = (int) Math.Ceiling( (float) imgW / (float) blockSize );
							var blockHCount = (int) Math.Ceiling( (float) imgH / (float) blockSize );

							for( int y = 0; y < blockHCount; y++ )
							{
								for( int x = 0; x < blockWCount; x++ )
								{
									// Now, we need to figure out the color for htis block
									// For this, we need to loop through the source pixels covered by it, and calculate the average color
									// The source pixel range...
									// For X: ( x * blockW ) => ( x * blockW ) + blockW
									// Example.. blockW = 3... x = 0
									// Another.. blockW = 3 .. x = 10 => 30 => 33
									// 0 => 3 where the top of the range is exclusive
									int x_start = x * blockSize;
									int x_end = x_start + blockSize;
									int y_start = y * blockSize;
									int y_end = y_start + blockSize;

									uint accumR = 0;
									uint accumG = 0;
									uint accumB = 0;
									uint accumA = 0;
									uint count = 0;

									for( int py = y_start; py < y_end && py < imgH; py++ )
									{
										for( int px = x_start; px < x_end && px < imgW; px++ )
										{
											long memOffset = ( ( py * imgW ) + px ) * 4L;
											accumR += sourceData[ memOffset ];
											accumG += sourceData[ memOffset + 1L ];
											accumB += sourceData[ memOffset + 2L ];
											accumA += sourceData[ memOffset + 3L ];

											count++;
										}
									}

									// Now, we figured out the color of our block, and we need to draw it on the panel
									g.FillRectangle(
										new SolidBrush( Color.FromArgb(
											( byte ) ( accumA / count ),
											( byte ) ( accumR / count ),
											( byte ) ( accumG / count ),
											( byte ) ( accumB / count ) ) ),
										new Rectangle( x, y, blockSize, blockSize ) 
									);
								}
							}
						}
					}

				}

				panel7.Invalidate();
			}
		}


	}
}
