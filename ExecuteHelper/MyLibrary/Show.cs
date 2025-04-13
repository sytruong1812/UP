using System;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace MyLibrary
{
    public class Show
    {
        [DllImport("kernel32")]
        static extern bool AllocConsole();
        public static void ShowCmd1()
        {
            AllocConsole();
            Console.WriteLine("Hello World!");
        }
        public static void ShowCmd2(int number)
        {
            AllocConsole();
            Console.WriteLine(number);
        }
        public static int ShowCmd3(string msg)
        {
            AllocConsole();
            Console.WriteLine(msg);
            return 0;
        }
        public static void ShowMsg1()
        {
            MessageBox.Show("Hello World!");
        }
        public static void ShowMsg2(int number)
        {
            MessageBox.Show($"Number = {number}");
        }
        public static int ShowMsg3(string msg)
        {
            MessageBox.Show(msg);
            return 0;
        }
    }
}


