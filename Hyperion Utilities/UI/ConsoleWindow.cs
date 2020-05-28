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
    public partial class ConsoleWindow : Form
    {
        public ConsoleWindow()
        {
            InitializeComponent();
        }

        public void Write( params string[] vs )
        {
            foreach( string o in vs )
            {
                // Cull out old lines we no longer need
                while( OutputBox.TextLength + o.Length > OutputBox.MaxLength )
                {
                    var linesToCull = Math.Min( OutputBox.Lines.Length / 2, 50 );
                    string[] newLines = new string[ OutputBox.Lines.Length - linesToCull ];
                    Array.ConstrainedCopy( OutputBox.Lines, linesToCull, newLines, 0, OutputBox.Lines.Length - linesToCull );
                    OutputBox.Lines = newLines;
                }

                OutputBox.AppendText( o );
            }

            
        }

        public void Clear()
        {
            OutputBox.Clear();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            OutputBox.AutoScrollOffset = new Point( 0, 50 );
            Core.Initialize();
        }

        private void EnterButton_Click(object sender, EventArgs e)
        {
            string input = InputBox.Text;
            InputBox.Clear();

            Core.OnCommand( input );
        }

        private void InputBox_KeyDown(object sender, KeyEventArgs e)
        {
            if( e.KeyCode == Keys.Enter )
            {
                string input = InputBox.Text;
                InputBox.Clear();

                Core.OnCommand( input );
            }
        }

        private void ConsoleWindow_FormClosing( object sender, FormClosingEventArgs e )
        {
            Core.Shutdown();
        }
    }
}
