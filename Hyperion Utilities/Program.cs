using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Hyperion
{
    static class Program
    {
        private static ConsoleWindow m_Form = null;
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            m_Form = new ConsoleWindow();
            Application.Run(m_Form);
        }

        public static ConsoleWindow GetWindow() { return m_Form; }
    }
}
