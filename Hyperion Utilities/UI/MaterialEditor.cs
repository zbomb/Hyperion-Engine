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
    public partial class MaterialEditor : Form
    {
        // Sub-Windows
        private KeyInput m_KeyInput;
        private TextureInput m_TextureInput;
        private string m_Path;
        private uint m_Identifier;
        private Material m_Material;
        private string m_LastKey;



        public MaterialEditor( string inPath, uint inId )
        {
            m_KeyInput      = null;
            m_TextureInput  = null;

            m_Path          = inPath;
            m_Identifier    = inId;
            m_Material      = null;
            m_LastKey       = "";

            InitializeComponent();
        }

        ~MaterialEditor()
        {

        }

        private void MaterialEditor_Shown( object sender, EventArgs e )
        {
            Core.WriteLine( "Opening material editor..." );
        }

        private void MaterialEditor_FormClosing( object sender, FormClosingEventArgs e )
        {
            Core.WriteLine( "Closing material editor..." );
        }

        private void MaterialEditor_Load( object sender, EventArgs e )
        {
            LoadSelectedFile();
        }

        private void LoadSelectedFile()
        {
            if( !MaterialManager.LoadMaterial( m_Path, out m_Material ) )
            {
                m_Material = null;
                Close();
            }
            else
            {
                // Update the UI with the data from the material file
                materialNameLabel.Text = m_Path;
                keyList.Items.Clear();

                foreach( var Entry in m_Material?.Properties )
                {
                    if( ( Entry.Key?.Length ?? 0 ) != 0 && Entry.Value != null )
                    {
                        keyList.Items.Add( Entry.Key );
                    }
                }
            }
        }


        private void WriteToFile()
        {
            if( ( m_Material?.Path?.Length ?? 0 ) == 0 )
            {
                Core.WriteLine( "[Warning] MaterialEditor: Failed to save material.. it was invalid?" );
                return;
            }

			if( !MaterialManager.SaveMaterial( m_Material ) )
			{
				Core.WriteLine( "[Warning] MaterialEditor: Failed to save material \"", m_Material.Path, "\"" );
			}
			else
			{
                Core.WriteLine( "=> Saved material \"", m_Material.Path, "\" to disk!" );
			}
        }

        private void ClearValueEditor()
        {
            KeyDisplayLabel.Text        = "Selected Key";
            ValueTypeBox.SelectedText   = "";
            boolSelector.Enabled        = false;
            boolSelector.Visible        = false;
            TextInput.Enabled           = false;
            TextInput.Visible           = false;
            BrowseButton.Enabled        = false;
            BrowseButton.Visible        = false;
        }

        private void saveButton_Click( object sender, EventArgs e )
        {
            // Commit the current value so its not lost
            if( m_LastKey != null )
            {
                CommitValue( m_LastKey );
            }

            WriteToFile();
            Close();
        }

        private void resetButton_Click( object sender, EventArgs e )
        {
            // Get the current key
            string prevKey = (string)keyList.SelectedItem;

            m_Material?.Properties?.Clear();
            keyList.Items.Clear();
            LoadSelectedFile();

            boolSelector.Enabled = false;
            boolSelector.Visible = false;
            TextInput.Enabled = false;
            TextInput.Visible = false;
            BrowseButton.Enabled = false;
            BrowseButton.Visible = false;

            // Now, were going to get the new value for 
            if( m_Material?.Properties?.ContainsKey( prevKey ) ?? false )
            {
                var newVar = m_Material.Properties[ prevKey ];
                var newType = newVar.GetType();

                if( newType == typeof( bool ) )
                {
                    ValueTypeBox.SelectedIndex = 0;
                    boolSelector.Enabled = true;
                    boolSelector.Visible = true;
                    boolSelector.Checked = (bool) newVar;
                }
                else if( newType == typeof( int ) )
                {
                    ValueTypeBox.SelectedIndex = 1;
                    TextInput.Enabled = true;
                    TextInput.Visible = true;
                    TextInput.Text = ( (int) newVar ).ToString();
                }
                else if( newType == typeof( uint ) )
                {
                    ValueTypeBox.SelectedIndex = 2;
                    TextInput.Enabled = true;
                    TextInput.Visible = true;
                    TextInput.Text = ( (uint) newVar ).ToString();
                }
                else if( newType == typeof( float ) )
                {
                    ValueTypeBox.SelectedIndex = 3;
                    TextInput.Enabled = true;
                    TextInput.Visible = true;
                    TextInput.Text = ( (float) newVar ).ToString( "0.000" );
                }
                else if( newType == typeof( string ) )
                {
                    ValueTypeBox.SelectedIndex = 4;
                    TextInput.Enabled = true;
                    TextInput.Visible = true;
                    TextInput.Text = (string) newVar;
                }
                else if( newType == typeof( TextureReference ) )
                {
                    ValueTypeBox.SelectedIndex = 5;
                    TextInput.Enabled = true;
                    TextInput.Visible = true;
                    BrowseButton.Enabled = true;
                    BrowseButton.Visible = true;

                    TextureReference tex = (TextureReference) newVar;
                    TextInput.Text = tex.Path;
                }
                else
                {
                    ValueTypeBox.SelectedIndex = 0;
                    boolSelector.Enabled = true;
                    boolSelector.Visible = true;
                    boolSelector.Checked = false;
                }

                KeyDisplayLabel.Text = prevKey + " [" + newType.Name + "]";
                keyList.SelectedItem = prevKey;
            }
            else
            {
                KeyDisplayLabel.Text        = "Selected Key";
                ValueTypeBox.SelectedItem   = null;
            }
        }


        private void cancelButton_Click( object sender, EventArgs e )
        {
            Close();
        }


        private void CommitValue( string inKey )
        {
            if( m_Material?.Properties == null )
            {
                Core.WriteLine( "[Warning] MaterialEditor: Failed to commit value, material (or properties) are null" );
                return;
            }

            if( inKey != null && inKey.Length > 0 )
            {
                object val = null;
                if( m_Material.Properties.TryGetValue( inKey, out val ) && val != null )
                {
                    var valType = val.GetType();
                    if( valType == typeof( bool ) )
                    {
                        m_Material.Properties[ inKey ] = (object) boolSelector.Checked;
                    }
                    else if( valType == typeof( int ) )
                    {
                        int newVal = 0;
                        if( !Int32.TryParse( TextInput.Text.Trim(), out newVal ) )
                        {
                            Core.WriteLine( "[Warning] MaterialEditor: Failed to apply value to \"", inKey, "\" because ", TextInput.Text, " is not a valid number!" );
                        }
                        else
                        {
                            m_Material.Properties[ inKey ] = (object) newVal;
                        }
                    }
                    else if( valType == typeof( uint ) )
                    {
                        uint newVal = 0;
                        if( !UInt32.TryParse( TextInput.Text.Trim(), out newVal ) )
                        {
                            Core.WriteLine( "[Warning] MaterialEditor: Failed to apply value to \"", inKey, "\" because ", TextInput.Text, " is not a valid positive number" );
                        }
                        else
                        {
                            m_Material.Properties[ inKey ] = (object) newVal;
                        }
                    }
                    else if( valType == typeof( float ) )
                    {
                        float newVal = 0.0f;
                        if( !Single.TryParse( TextInput.Text.Trim(), out newVal ) )
                        {
                            Core.WriteLine( "[Warning] MaterialEditor: Failed to apply value to \"", inKey, "\" because ", TextInput.Text, " is not a valid decimal number" );
                        }
                        else
                        {
                            m_Material.Properties[ inKey ] = (object) newVal;
                        }
                    }
                    else if( valType == typeof( string ) )
                    {
                        var newVal = TextInput.Text;
                        if( newVal == null ) { newVal = ""; }

                        m_Material.Properties[ inKey ] = (object) newVal;
                    }
                    else if( valType == typeof( TextureReference ) )
                    {
                        TextureReference tex = new TextureReference();
                        tex.Path = null;

                        var path = TextInput.Text.Trim();
                        if( !path.EndsWith( ".htx" ) )
                        {
                            Core.WriteLine( "[Warning] MaterialEditor: Failed to apply value to \"", inKey, "\" because ", path, " isnt a valid texture file" );
                        }
                        else
                        {
                            tex.Path = path;
                            tex.Hash = Core.CalculateAssetIdentifier( path );

                            if( tex.Path == null )
                            {
                                Core.WriteLine( "[Warning] MaterialEditor: Failed to apply value to \"", inKey, "\" because ", path, " isnt a valid texture file" );
                            }
                            else
                            {
                                m_Material.Properties[ inKey ] = (object) tex;
                            }
                        }
                    }
                    else
                    {
                        Core.WriteLine( "[Warning] MaterialEditor: Failed to apply value to \"", inKey, "\" because the value type (", valType.ToString(), ") is invalid" );
                    }
                }
                else
                {
                    Core.WriteLine( "[Warning] MaterialEditor: Failed to commit value to \"", inKey, "\" because this is not a valid key in this material" );
                }
            }
            else
            {
                Core.WriteLine( "[Warning] MaterialEditor: Attempt to commit a null/invalid key!" );
            }
        }


        private void keyList_SelectedValueChanged( object sender, EventArgs e )
        {
            if( m_Material?.Properties == null )
            {
                return;
            }

            if( keyList.SelectedItem == null )
            {
                ClearValueEditor();
                if( m_LastKey != null && m_Material.Properties.ContainsKey( m_LastKey ) )
                {
                    CommitValue( m_LastKey );
                }

                return;
            }

            if( m_LastKey != (string)keyList.SelectedItem )
            {
                if( m_LastKey != null && m_Material.Properties.ContainsKey( m_LastKey ) )
                {
                    CommitValue( m_LastKey );
                }

                m_LastKey = (string) keyList.SelectedItem;

            }
            else
            {
                return;
            }

            // We need to get the key that is selected
            var targetValue = keyList.SelectedItem as string;
            if( targetValue == null || targetValue.Length == 0 )
            {
                ClearValueEditor();
                return;
            }

            // Now find the entry associated with this key
            object valueObj = null;
            if( !m_Material.Properties.TryGetValue( targetValue, out valueObj ) || valueObj == null )
            {
                ClearValueEditor();
                return;
            }

            // And now cast it to the target type
            var valueType = valueObj.GetType();
            KeyDisplayLabel.Text = targetValue + " [" + valueType.Name + "]";

            // Hide other controls
            boolSelector.Enabled    = false;
            boolSelector.Visible    = false;
            TextInput.Enabled       = false;
            TextInput.Visible       = false;
            BrowseButton.Enabled    = false;
            BrowseButton.Visible    = false;

            if( valueType == typeof( bool ) )
            {
                ValueTypeBox.SelectedIndex = 0;

                boolSelector.Enabled = true;
                boolSelector.Visible = true;
                boolSelector.Checked = (bool)valueObj;
            }
            else if( valueType == typeof( int ) )
            {
                ValueTypeBox.SelectedIndex = 1;

                TextInput.Enabled = true;
                TextInput.Visible = true;
                TextInput.Text = ( (int) valueObj ).ToString();
            }
            else if( valueType == typeof( uint ) )
            {
                ValueTypeBox.SelectedIndex = 2;

                TextInput.Enabled = true;
                TextInput.Visible = true;
                TextInput.Text = ( (uint) valueObj ).ToString();
            }
            else if( valueType == typeof( float ) )
            {
                ValueTypeBox.SelectedIndex = 3;

                TextInput.Enabled = true;
                TextInput.Visible = true;
                float val = (float) valueObj;
                if( val < 0.001f && val > -0.001f ) { val = 0.0f; }
                TextInput.Text = val.ToString( "0.000" );
            }
            else if( valueType == typeof( string ) )
            {
                ValueTypeBox.SelectedIndex = 4;

                TextInput.Enabled = true;
                TextInput.Visible = true;
                TextInput.Text = (string) valueObj;
            }
            else if( valueType == typeof( TextureReference ) )
            {
                ValueTypeBox.SelectedIndex = 5;

                TextInput.Enabled = false;
                TextInput.Visible = true;
                BrowseButton.Enabled = true;
                BrowseButton.Visible = true;

                TextInput.Text = ( (TextureReference) valueObj ).Path;
            }
            else
            {
                Core.WriteLine( "[ERROR] MaterialEditor: Failed to create value editor menu, invalid value type \"", valueType.Name, "\"" );
                ClearValueEditor();
            }
        }

        private void ValueTypeBox_SelectedIndexChanged( object sender, EventArgs e )
        {
            // First, hide the value selectors
            boolSelector.Enabled = false;
            boolSelector.Visible = false;
            TextInput.Enabled = false;
            TextInput.Visible = false;
            BrowseButton.Enabled = false;
            BrowseButton.Visible = false;

            // Then, figure out which one to use, and create the proper one, and set it to the default value
            // First though, we want to commit the value we had before changing the selection
            var targetValue = keyList.SelectedItem as string;
            if( targetValue != null && targetValue.Length > 0 )
            {
                // If the type of the value is the same as the set one, use the set value
                object def_value = null;
                if( !m_Material.Properties.TryGetValue( targetValue, out def_value ) ) { def_value = null; }

                // Now, we want to get the new target value type
                Type varType = typeof( void );
                object value = null;

                switch( ValueTypeBox.SelectedIndex )
                {
                    case 0:
                        varType = typeof( bool );
                        boolSelector.Enabled = true;
                        boolSelector.Visible = true;
                        boolSelector.Checked = false;
                        value = ( def_value != null ? def_value.GetType() == typeof( bool ) ? (bool) def_value : false : false );
                        break;
                    case 1:
                        varType = typeof( int );
                        TextInput.Enabled = true;
                        TextInput.Visible = true;
                        TextInput.Text = "0";
                        value = ( def_value != null ? def_value.GetType() == typeof( int ) ? (int) def_value : 0 : 0 );
                        break;
                    case 2:
                        varType = typeof( uint );
                        TextInput.Enabled = true;
                        TextInput.Visible = true;
                        TextInput.Text = "0";
                        value = ( def_value != null ? def_value.GetType() == typeof( uint ) ? (uint) def_value : 0 : 0 );
                        break;
                    case 3:
                        varType = typeof( float );
                        TextInput.Enabled = true;
                        TextInput.Visible = true;
                        TextInput.Text = "0.0";
                        value = ( def_value != null ? def_value.GetType() == typeof( float ) ? (float) def_value : 0.0f : 0.0f );
                        break;
                    case 4:
                        varType = typeof( string );
                        TextInput.Enabled = true;
                        TextInput.Visible = true;
                        TextInput.Text = def_value as string ?? "";
                        value = ( def_value as string ?? "" );
                        break;
                    case 5:
                        varType = typeof( TextureReference );
                        TextInput.Enabled = true;
                        TextInput.Visible = true;
                        BrowseButton.Enabled = true;
                        BrowseButton.Visible = true;
                        TextureReference tref = ( def_value != null ? def_value.GetType() == typeof( TextureReference ) ? (TextureReference) def_value : new TextureReference() : new TextureReference() );
                        TextInput.Text = tref.Path ?? "";
                        value = tref;
                        break;
                    default:
                        break;
                }

                // Now, update the key label
                KeyDisplayLabel.Text = targetValue + " [" + varType.Name + "]";

                // Finally, commit the correct type to the list
                if( def_value != null )
                {
                    if( m_Material.Properties.ContainsKey( targetValue ) )
                    {
                        m_Material.Properties[ targetValue ] = value;
                    }
                    else
                    {
                        Core.WriteLine( "[ERROR] MaterialEditor: Failed to change data type for \"", targetValue, "\"!" );
                        Close();
                    }
                }
            }
        }

        private void AddButton_Click( object sender, EventArgs e )
        {
            // Open dialog to get a key name from the user
            if( m_KeyInput != null || m_Material?.Properties == null )
            {
                return;
            }

            m_KeyInput = new KeyInput();
            if( m_KeyInput.ShowDialog( Program.GetWindow() ) == DialogResult.OK )
            {
                var result = m_KeyInput.InputBox.Text?.ToLower();
                if( result == null || result.Length == 0 )
                {
                    Core.WriteLine( "[Warning] MaterialEditor: Failed to add new key, invalid keyname!" );
                    return;
                }

                // Now lets ensure this key doesnt already exist
                if( m_Material.Properties.ContainsKey( result ) )
                {
                    Core.WriteLine( "[Warning] MaterialEditor: Failed to add key new \"", result, "\" because it already exists" );
                    return;
                }

                m_Material.Properties[ result ] = (object) false;

                // Now that we added it to the data structure, we need to update the key list, and select it
                keyList.Items.Add( result );
                keyList.SetSelected( keyList.Items.Count - 1, true );
            }

            m_KeyInput.Dispose();
            m_KeyInput = null;
        }

        private void RemoveButton_Click( object sender, EventArgs e )
        {
            if( m_Material?.Properties == null )
            {
                return;
            }

            // Get the currently selected key
            string key = (string)keyList.SelectedItem;
            if( key == null || key.Length == 0 )
            {
                Core.WriteLine( "[Warning] MaterialEditor: Failed to remove key, there was no valid key selected" );
                return;
            }

            // Ensure this key is valid
            if( !m_Material.Properties.ContainsKey( key ) || !m_Material.Properties.Remove( key ) )
            {
                Core.WriteLine( "[Warning] MaterialEditor: Failed to properly remove key \"", key, "\" because it wasnt found in the material" );
            }

            // Now, we need to remove the selected key from the materail, and the UI, and select another one
            keyList.Items.Remove( key );
            if( keyList.Items.Count > 0 )
            {
                keyList.SetSelected( 0, true );
            }
        }

        private void RenameButton_Click( object sender, EventArgs e )
        {
            if( m_KeyInput != null || m_Material?.Properties == null )
            {
                return;
            }

            string currentKey = (string)keyList.SelectedItem;
            if( currentKey == null || currentKey.Length == 0 )
            {
                return;
            }

            m_KeyInput = new KeyInput();
            if( m_KeyInput.ShowDialog( Program.GetWindow() ) == DialogResult.OK )
            {
                // Get the result of the dialog box
                var result = m_KeyInput.InputBox.Text?.ToLower();
                if( result == null || result.Length == 0 )
                {
                    Core.WriteLine( "[Warning] MaterialEditor: Failed to rename key \"", currentKey, "\" because the new key name was invalid" );
                    return;
                }

                // Ensure this key isnt already in use
                if( m_Material.Properties.ContainsKey( result ) )
                {
                    Core.WriteLine( "[Warning] MaterialEditor: Failed to rename key to \"", result, "\" because this key already exists" );
                    return;
                }

                // Find the index of the currently selected index
                int currentIndex = keyList.SelectedIndex;
                if( currentIndex < 0 )
                {
                    Core.WriteLine( "[Warning] MaterialEditor: Failed to rename key to \"", result, "\" because the UI couldnt be updated" );
                    return;
                }

                // Update the material structure
                m_Material.Properties[ result ] = m_Material.Properties[ currentKey ];
                m_Material.Properties.Remove( currentKey );

                // Update the UI
                keyList.Items[ currentIndex ] = (object) result;
                KeyDisplayLabel.Text = result + " [" + m_Material.Properties[ result ].GetType().Name + "]";
            }

            m_KeyInput.Dispose();
            m_KeyInput = null;
        }

        private void BrowseButton_Click( object sender, EventArgs e )
        {
            // We need to open a dialog, allow us to select one of the textures we have registered in the manifest
            if( m_TextureInput != null || m_Material?.Properties == null )
            {
                return;
            }

            // Open promt to select texture
            m_TextureInput = new TextureInput();
            if( m_TextureInput.ShowDialog( Program.GetWindow() ) == DialogResult.OK )
            {
                string selectedTexture = m_TextureInput.GetSelectedTexture();
                if( selectedTexture != null && selectedTexture.Length > 0 )
                {
                    if( TextInput.Visible )
                    {
                        // Lets validate the texture
                        if( File.Exists( "content/" + selectedTexture ) && selectedTexture.EndsWith( ".htx" ) )
                        {
                            TextInput.Text = selectedTexture;
                            TextInput.Enabled = false;
                        }
                        else
                        {
                            Core.WriteLine( "[Warning] MaterialEditor:  (", selectedTexture, ")" );
                        }
                    }
                }
            }

            m_TextureInput.Dispose();
            m_TextureInput = null;
        }
    }
}
