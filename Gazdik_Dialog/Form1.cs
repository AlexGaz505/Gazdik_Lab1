using System;
using System.Diagnostics;
using System.Threading;
using System.Windows.Forms;

namespace Gazdik_Dialog
{
    public partial class Form1 : Form
    {
        private Process? consoleApp = null;

        private EventWaitHandle startSignal   = new EventWaitHandle(false, EventResetMode.AutoReset, "Gazdik_Evt_Start");
        private EventWaitHandle stopSignal    = new EventWaitHandle(false, EventResetMode.AutoReset, "Gazdik_Evt_Stop");
        private EventWaitHandle exitSignal    = new EventWaitHandle(false, EventResetMode.AutoReset, "Gazdik_Evt_Exit");
        private EventWaitHandle confirmSignal = new EventWaitHandle(false, EventResetMode.AutoReset, "Gazdik_Evt_Confirm");

        private int activeThreadsCount = 0;

        public Form1()
        {
            InitializeComponent();
        }

        private void RunConsoleIfClosed()
        {
            if (consoleApp == null || consoleApp.HasExited)
            {
                consoleApp = new Process();
                consoleApp.StartInfo.FileName = @"..\..\..\..\..\x64\Debug\Gazdik_Console.exe";
                consoleApp.EnableRaisingEvents = true;
                consoleApp.SynchronizingObject = this;

                consoleApp.Exited += (s, e) => {
                    listSessions.Items.Clear();
                    activeThreadsCount = 0;
                    consoleApp = null;
                };

                consoleApp.Start();

                listSessions.Items.Clear();
                listSessions.Items.Add("Все потоки");
                listSessions.Items.Add("Главный поток");
                activeThreadsCount = 0;
            }
        }

        private void StartButton_Click(object sender, EventArgs e)
        {
            RunConsoleIfClosed();

            int n = (int)numberThreads.Value;
            for (int i = 0; i < n; i++)
            {
                startSignal.Set();
                if (confirmSignal.WaitOne(2000))
                {
                    activeThreadsCount++;
                    listSessions.Items.Add($"Поток №{activeThreadsCount}");
                }
            }
        }

        private void StopButton_Click(object sender, EventArgs e)
        {
            if (consoleApp == null || consoleApp.HasExited) return;

            stopSignal.Set();
            if (confirmSignal.WaitOne(2000))
            {
                if (activeThreadsCount > 0)
                {
                    listSessions.Items.RemoveAt(listSessions.Items.Count - 1);
                    activeThreadsCount--;
                }
            }
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (consoleApp != null && !consoleApp.HasExited)
            {
                exitSignal.Set();
                confirmSignal.WaitOne(1000);
            }
        }
    }
}
