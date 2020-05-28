using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Hyperion
{

	public partial class TextureImportOptions : Form
	{
		private TextureImporter m_ParentWindow;

		private long ApproximateOutputImageSize( uint inWidth, uint inHeight, TextureFormat inFormat )
		{
			switch( inFormat )
			{
				case TextureFormat.RGBA_8BIT_UNORM:
				case TextureFormat.RGBA_8BIT_UNORM_SRGB:
					return ( inWidth * inHeight * 4L );
				case TextureFormat.RG_8BIT_UNORM:
					return ( inWidth * inHeight * 2L );
				case TextureFormat.R_8BIT_UNORM:
					return ( inWidth * inHeight );
				case TextureFormat.RGBA_BC_7:
				case TextureFormat.RGB_BC_7:
				case TextureFormat.RGBA_DXT_5:

					// First, figure out how many blocks will be used for this texture
					// Each block is 4x4 pixels, and takes up 16 bytes
					var widthBlocks = ( ( inWidth / 4 ) + ( ( ( inWidth % 4 ) == 0 ) ? 0 : 1 ) );
					var heightBlocks = ( ( inHeight / 4 ) + ( ( ( inHeight % 4 ) == 0 ) ? 0 : 1 ) );

					return ( widthBlocks * heightBlocks * 16L );

				case TextureFormat.RGB_DXT_1:

					// This algorithm is like BC7 & DXT5, but instead of 16 bytes per block, it uses 8 bytes
					// This is a more low quality compression algorithm, and can only handle 3 channels
					var wb = ( ( inWidth / 4 ) + ( ( ( inWidth % 4 ) == 0 ) ? 0 : 1 ) );
					var hb = ( ( inHeight / 4 ) + ( ( ( inHeight % 4 ) == 0 ) ? 0 : 1 ) );

					return ( wb * hb * 8L );

				case TextureFormat.NONE:
				default:
					return 0L;
			}
		}

		public UI.TextureImportLODSettings[] m_ImportSettings;

		public TextureImportOptions( TextureImporter inParent )
		{
			if( inParent == null || !inParent.Visible )
			{
				Core.WriteLine( "[ERROR] TextureImporter: Failed to open import options dialog.. invalid parameters" );

				DialogResult = DialogResult.Abort;
				Close();
			}
			else
			{
				DialogResult = DialogResult.Cancel;
				m_ParentWindow = inParent;
				InitializeComponent();
			}

			filenameLabel.Text = ( "Output File: " + ( State.Path ?? "[Invalid]" ) );
			lodCountLabel.Text = ( "LOD Count: " + State.LODs.Count.ToString() );
			formatLabel.Text = ( "Format: " + Enum.GetName( typeof( TextureFormat ), State.Format ) );

			if( State.LODs.Count > 0 )
			{
				fullResLabel.Text = ( "Full Res: " + State.LODs[ 0 ].Width.ToString() + "x" + State.LODs[ 0 ].Height.ToString() );
			}

			m_ImportSettings = new UI.TextureImportLODSettings[ State.LODs.Count ];

			// We need to approximate the output image size based on each LOD index, and create their controls
			long totalSize = 0;
			for( int i = 0; i < State.LODs.Count; i++ )
			{
				var lod = State.LODs[ i ];
				totalSize += ApproximateOutputImageSize( lod.Width, lod.Height, State.Format );

				m_ImportSettings[ i ] = new UI.TextureImportLODSettings( ( byte ) i, lod.Width, lod.Height, lod.SourceFormat, State.Format );
				lodList.Controls.Add( m_ImportSettings[ i ] );
			}

			sizeLabel.Text = "Size: " + ( totalSize / 1024L ).ToString() + "kb";
		}


		private TextureImportState State => m_ParentWindow.state;

		private void listView1_SelectedIndexChanged( object sender, EventArgs e )
		{
			// TODO: Remove
		}

		private void lodList_Paint( object sender, PaintEventArgs e )
		{
			// TODO: Remove
		}

		private void cancelButton_Click( object sender, EventArgs e )
		{
			// Set the result and close the form
			DialogResult = DialogResult.Cancel;
			Close();
		}

		private void importButton_Click( object sender, EventArgs e )
		{
			// Lets ensure everything is selected properly
			for( int i = 0; i < m_ImportSettings.Length; i++ )
			{
				// Check if this LOD needs a color option to be selected
				if( m_ImportSettings[ i ].colorOptions.Enabled )
				{
					if( m_ImportSettings[ i ].colorOptions.SelectedIndex < 0 ||
						m_ImportSettings[ i ].colorOptions.SelectedItem == null ||
						( string ) m_ImportSettings[ i ].colorOptions.SelectedItem == "Required" )
					{
						MessageBox.Show( "LOD " + i.ToString() + " has an invalid color option selected", "Invalid LOD setting", MessageBoxButtons.OK, MessageBoxIcon.Warning );
						return;
					}
				}

				if( m_ImportSettings[ i ].alphaOptions.Enabled )
				{
					if( m_ImportSettings[ i ].alphaOptions.SelectedIndex < 0 ||
						m_ImportSettings[ i ].alphaOptions.SelectedItem == null ||
						( string ) m_ImportSettings[ i ].alphaOptions.SelectedItem == "Required" )
					{
						MessageBox.Show( "LOD " + i.ToString() + " has an invalid alpha option selected", "Invalid LOD setting", MessageBoxButtons.OK, MessageBoxIcon.Warning );
						return;
					}
				}
			}

			// Set the result, and we also need to pass back the options we selected
			DialogResult = DialogResult.OK;
			Close();

		}
	}
}
