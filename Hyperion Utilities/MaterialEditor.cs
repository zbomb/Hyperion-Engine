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
    struct TextureReference
    {
        string Path;
        uint Hash;
    }

    struct MaterialData
    {
        string Path;
        Dictionary< string, object > Data;
    }

    public partial class MaterialEditor : Form
    {
        private FileStream m_File;

        public MaterialEditor( FileStream inFile )
        {
            m_File = inFile;
            InitializeComponent();
        }

        ~MaterialEditor()
        {
            if( m_File != null )
            {
                m_File.Dispose();
                m_File = null;
            }
        }

        private void MaterialEditor_Shown( object sender, EventArgs e )
        {
            Core.WriteLine( "Opening material editor..." );
        }

        private void MaterialEditor_FormClosing( object sender, FormClosingEventArgs e )
        {
            Core.WriteLine( "Closing material editor..." );

            // Try to close the file, incase the destructor doesnt run until application close
            if( m_File != null )
            {
                m_File.Dispose();
                m_File = null;
            }
        }

        private void MaterialEditor_Load( object sender, EventArgs e )
        {
            if( m_File == null )
            {
                Core.WriteLine( "[ERROR] MaterialEditor: Failed to load.. there is no associated file!" );
                return;
            }

            // First, lets set the title text to the file name
            materialNameLabel.Text = m_File.Name;

            // Next, we need to actually read in the file, into a structure we can edit
            // TODO: We also need a system in place that manages the physical asset manifest
            // This way, we can find the texture path from the identifier baked into the material
            
        }
    }
}
