using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;


namespace Hyperion
{
    public partial class TextureInput : Form
    {
        public TextureInput()
        {
            InitializeComponent();
        }

        private void TextureInput_Load( object sender, EventArgs e )
        {
            TextureList.Items.Clear();

            // We want to discover all texture assets and add them to the list
            foreach( var entry in AssetIdentifierCache.GetList() )
			{
                if( entry.Value.EndsWith( ".htx" ) )
				{
                    TextureList.Items.Add( entry.Value );
				}
			}
        }

        private void SelectButton_Click( object sender, EventArgs e )
        {
            DialogResult = DialogResult.OK;
            Close();
        }

        private void CancelButton_Click( object sender, EventArgs e )
        {
            DialogResult = DialogResult.Cancel;
            Close();
        }

        public string GetSelectedTexture()
        {
            return (string)TextureList.SelectedItem;
        }
    }
}
