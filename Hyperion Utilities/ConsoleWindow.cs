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
                OutputBox.Text += o;
            }
        }

        public void Clear()
        {
            OutputBox.Clear();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            Hyperion.Core.Initialize();
        }

        private void EnterButton_Click(object sender, EventArgs e)
        {
            string input = InputBox.Text;
            InputBox.Clear();

            Hyperion.Core.OnCommand(input);
        }

        private void InputBox_KeyDown(object sender, KeyEventArgs e)
        {
            if( e.KeyCode == Keys.Enter )
            {
                string input = InputBox.Text;
                InputBox.Clear();

                Hyperion.Core.OnCommand(input);
            }
        }
    }
}
