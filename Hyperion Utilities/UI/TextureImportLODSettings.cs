using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Hyperion.UI
{
	public struct ColorChangeParameters
	{
		public bool r;
		public bool g;
		public bool b;
	}


	public partial class TextureImportLODSettings : UserControl
	{
		private uint m_Index;

		private enum ConvertType
		{
			Increase,
			Decrease,
			Same
		}

		public ColorChangeParameters m_ColorParam;
		public byte m_AlphaParam;

		private bool IsGrayscaleSource( ImageFormat inSource )
		{
			switch( inSource )
			{
				case ImageFormat.GS:
				case ImageFormat.GSA:
					return true;
				default:
					return false;
			}
		}

		private bool IsGrayscaleOutput( TextureFormat inOut )
		{
			switch( inOut )
			{
				case TextureFormat.R_8BIT_UNORM:
				case TextureFormat.RG_8BIT_UNORM:
					return true;
				default:
					return false;
			}
		}

		private bool IsColorSource( ImageFormat inSource )
		{
			switch( inSource )
			{
				case ImageFormat.RGB:
				case ImageFormat.RGBA:
					return true;
				default:
					return false;
			}
		}

		private bool IsColorOutput( TextureFormat inOut )
		{
			switch( inOut )
			{
				case TextureFormat.RGB_BC_7:
				case TextureFormat.RGB_DXT_1:
				case TextureFormat.RGBA_8BIT_UNORM:
				case TextureFormat.RGBA_8BIT_UNORM_SRGB:
				case TextureFormat.RGBA_BC_7:
				case TextureFormat.RGBA_DXT_5:
					return true;
				default:
					return false;
			}
		}

		private bool IsAlphaInput( ImageFormat inSource )
		{
			switch( inSource )
			{
				case ImageFormat.GSA:
				case ImageFormat.RGBA:
					return true;
				default:
					return false;
			}
		}

		private bool IsAlphaOutput( TextureFormat inOut )
		{
			switch( inOut )
			{
				case TextureFormat.RGBA_8BIT_UNORM:
				case TextureFormat.RGBA_8BIT_UNORM_SRGB:
				case TextureFormat.RGBA_BC_7:
				case TextureFormat.RGBA_DXT_5:
				case TextureFormat.RG_8BIT_UNORM:
					return true;
				default:
					return false;
			}
		}

		private ConvertType GetColorConvertType( ImageFormat inSource, TextureFormat inOutput )
		{
			if( IsGrayscaleSource( inSource ) )
			{
				if( IsColorOutput( inOutput ) )
				{
					return ConvertType.Increase;
				}
				else
				{
					return ConvertType.Same;
				}
			}
			else // Color source
			{
				if( IsColorOutput( inOutput ) )
				{
					return ConvertType.Same;
				}
				else
				{
					return ConvertType.Decrease;
				}
			}
		}

		private ConvertType GetAlphaConvertType( ImageFormat inSource, TextureFormat inOutput )
		{
			if( IsAlphaInput( inSource ) )
			{
				if( IsAlphaOutput( inOutput ) )
				{
					return ConvertType.Same;
				}
				else
				{
					return ConvertType.Decrease;
				}
			}
			else // Input no alpha
			{
				if( IsAlphaOutput( inOutput ) )
				{
					return ConvertType.Increase;
				}
				else
				{
					return ConvertType.Same;
				}
			}
		}

		public TextureImportLODSettings( byte inIndex, uint inWidth, uint inHeight, ImageFormat sourceFormat, TextureFormat outputFormat )
		{
			m_Index = inIndex;
			m_ColorParam.r = false;
			m_ColorParam.g = false;
			m_ColorParam.b = false;
			m_AlphaParam = 0;

			InitializeComponent();

			indexLabel.Text = ( "LOD Index: [" + m_Index.ToString() + "]" );
			sourceLabel.Text = ( "Source: " + inWidth.ToString() + "x" + inHeight.ToString() + " [" + Enum.GetName( typeof( ImageFormat ), sourceFormat ) + "]" );

			// First, lets determine if were increasing the number of colors channels, decreasing, or staying the same
			// Also, do the same for alpha, then we can setup the controls properly
			var colorChange = GetColorConvertType( sourceFormat, outputFormat );
			var alphaChange = GetAlphaConvertType( sourceFormat, outputFormat );

			var tip = new ToolTip
			{
				AutoPopDelay = 5000,
				InitialDelay = 1000,
				ReshowDelay = 500,
				ShowAlways = true
			};

			switch( colorChange )
			{
				case ConvertType.Same:
					colorOptions.Items.Clear();
					colorOptions.Text = "N/A";
					colorOptions.Enabled = false;
					break;

				case ConvertType.Increase: // Going from grayscale to RGB
					colorOptions.Items.AddRange(
						new string[]{
						"R",
						"G",
						"B",
						"RG",
						"RB",
						"GB",
						"RGB" }
						);
					colorOptions.SelectedIndex = -1;
					colorOptions.Text = "Required";
					colorOptions.Enabled = true;
					tip.SetToolTip( colorOptions, "Select which channels get 'gray' value from source" );
					break;

				case ConvertType.Decrease: // Going from RGB to Grayscale
					colorOptions.Items.AddRange(
						new string[]
						{
							"R",
							"G",
							"B",
							"RG",
							"RB",
							"GB",
							"RGB"
						} );
					colorOptions.SelectedIndex = -1;
					colorOptions.Text = "Required";
					colorOptions.Enabled = true;
					tip.SetToolTip( colorOptions, "Select which channel(s) to use when calculating 'gray' output value (uses average if multiple channels selected)" );
					break;
			}

			switch( alphaChange )
			{
				case ConvertType.Same:
					alphaOptions.Items.Clear();
					alphaOptions.Text = "N/A";
					alphaOptions.Enabled = false;
					break;

				case ConvertType.Increase: // Going from no alpha, to alpha
					alphaOptions.Items.AddRange(
						new string[]
						{
							"Full Alpha",
							"Zero Alpha"
						} );
					alphaOptions.SelectedIndex = -1;
					alphaOptions.Text = "Required";
					alphaOptions.Enabled = true;
					tip.SetToolTip( alphaOptions, "What should the output alpha channel be?" );
					break;

				case ConvertType.Decrease: // Going from alpha to no alpha
					alphaOptions.Items.AddRange(
						new string[]
						{
							"Ignore Alpha"
						} );
					alphaOptions.Enabled = true;
					alphaOptions.SelectedIndex = 0;
					alphaOptions.Text = "Ignore Alpha";
					tip.SetToolTip( alphaOptions, "What should we do with the source alpha channel?" );
					break;
			}
		}

		private void colorOptions_SelectedIndexChanged( object sender, EventArgs e )
		{
			m_ColorParam.r = false;
			m_ColorParam.g = false;
			m_ColorParam.b = false;

			// We ned to figure out whats selected and set it as a param
			switch( colorOptions.SelectedItem )
			{
				case "R":
					m_ColorParam.r = true;
					break;
				case "G":
					m_ColorParam.g = true;
					break;
				case "B":
					m_ColorParam.b = true;
					break;
				case "RG":
					m_ColorParam.r = true;
					m_ColorParam.g = true;
					break;
				case "RB":
					m_ColorParam.r = true;
					m_ColorParam.g = true;
					break;
				case "GB":
					m_ColorParam.g = true;
					m_ColorParam.b = true;
					break;
				case "RGB":
					m_ColorParam.r = true;
					m_ColorParam.g = true;
					m_ColorParam.b = true;
					break;
				default:
					return;
			}
		}

		private void alphaOptions_SelectedIndexChanged( object sender, EventArgs e )
		{
			m_AlphaParam = 0;

			switch( alphaOptions.SelectedItem )
			{
				case "Full Alpha":
					m_AlphaParam = 255;
					break;
				case "Zero Alpha":
					m_AlphaParam = 0;
					break;
				default:
					m_AlphaParam = 0;
					break;
			}
		}
	}
}
