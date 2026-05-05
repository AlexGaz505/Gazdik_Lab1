// ============================================================
// Form1.cs — логика главного окна (C# WinForms)
//
// Это "диспетчер" — GUI часть программы.
// Она запускает C++ консоль как дочерний процесс и управляет ею
// через именованные Win32 Events (те же имена что в C++!).
//
// Как это работает в целом:
//   1. Пользователь нажимает Start
//   2. C# устанавливает событие "Gazdik_Evt_Start"
//   3. C++ консоль просыпается, создаёт поток
//   4. C++ устанавливает "Gazdik_Evt_Confirm"
//   5. C# получает подтверждение и обновляет список
// ============================================================

using System;
using System.Diagnostics;
using System.Threading;
using System.Windows.Forms;

namespace Gazdik_Dialog
{
    public partial class Form1 : Form
    {
        // -------------------------------------------------------
        // Поле для хранения ссылки на запущенный C++ процесс
        // ? означает что переменная может быть null
        // -------------------------------------------------------
        private Process? consoleApp = null;

        // -------------------------------------------------------
        // Именованные события — "телефонные линии" с C++ процессом
        //
        // EventWaitHandle — C# обёртка над Win32 Event
        //   false            — изначально не сигнальное
        //   EventResetMode.AutoReset — само сбрасывается после срабатывания
        //   "Gazdik_Evt_..." — ИМЯ события (должно совпадать с C++!)
        //
        // Когда мы вызываем Set() — это как нажать кнопку звонка.
        // C++ ловит его через WaitForMultipleObjects.
        // Когда C++ вызывает SetEvent(confirmEvt) —
        //   мы ловим это через confirmSignal.WaitOne().
        // -------------------------------------------------------
        private EventWaitHandle startSignal =
            new EventWaitHandle(false, EventResetMode.AutoReset, "Gazdik_Evt_Start");

        private EventWaitHandle stopSignal =
            new EventWaitHandle(false, EventResetMode.AutoReset, "Gazdik_Evt_Stop");

        private EventWaitHandle exitSignal =
            new EventWaitHandle(false, EventResetMode.AutoReset, "Gazdik_Evt_Exit");

        private EventWaitHandle confirmSignal =
            new EventWaitHandle(false, EventResetMode.AutoReset, "Gazdik_Evt_Confirm");

        // Счётчик активных потоков (для нумерации в списке)
        private int activeThreadsCount = 0;

        // Конструктор формы — вызывается при создании окна
        public Form1()
        {
            // InitializeComponent() — сгенерированный код из Form1.Designer.cs
            // Он создаёт все кнопки, список и т.д. по описанию из Designer
            InitializeComponent();
        }

        // -------------------------------------------------------
        // RunConsoleIfClosed — запустить C++ процесс если он не запущен
        // -------------------------------------------------------
        private void RunConsoleIfClosed()
        {
            // Проверяем: процесс не существует ИЛИ уже завершился
            if (consoleApp == null || consoleApp.HasExited)
            {
                consoleApp = new Process();

                // Путь к .exe — относительный от папки Gazdik_Dialog.exe
                // ..\..\..\..\  — поднимаемся на 4 уровня вверх от bin\Debug\net9.0-windows\
                // x64\Debug\    — папка с C++ Debug сборкой
                consoleApp.StartInfo.FileName = @"..\..\..\..\Gazdik_Console\x64\Debug\Gazdik_Console.exe";

                // Хотим получать событие когда процесс завершится
                consoleApp.EnableRaisingEvents = true;

                // SynchronizingObject = this означает: вызывать обработчики
                // событий в потоке UI (иначе нельзя трогать GUI из другого потока)
                consoleApp.SynchronizingObject = this;

                // Что делать когда C++ процесс закрылся
                consoleApp.Exited += (s, e) => {
                    listSessions.Items.Clear();     // очищаем список
                    activeThreadsCount = 0;
                    consoleApp = null;
                };

                consoleApp.Start();  // запускаем!

                // Заполняем список начальными пунктами
                listSessions.Items.Clear();
                listSessions.Items.Add("Все потоки");
                listSessions.Items.Add("Главный поток");
                activeThreadsCount = 0;
            }
        }

        // -------------------------------------------------------
        // Кнопка Start — создать N новых рабочих потоков
        // -------------------------------------------------------
        private void StartButton_Click(object sender, EventArgs e)
        {
            RunConsoleIfClosed();  // убеждаемся что C++ работает

            // Читаем N из числового поля (сколько потоков создать)
            int n = (int)numberThreads.Value;

            for (int i = 0; i < n; i++)
            {
                // Сигналим C++: "создай поток"
                startSignal.Set();

                // Ждём подтверждения максимум 2 секунды
                // WaitOne(2000) возвращает:
                //   true  — подтверждение пришло вовремя
                //   false — таймаут (что-то пошло не так)
                if (confirmSignal.WaitOne(2000))
                {
                    activeThreadsCount++;
                    listSessions.Items.Add($"Поток №{activeThreadsCount}");
                }
            }
        }

        // -------------------------------------------------------
        // Кнопка Stop — остановить последний поток
        // -------------------------------------------------------
        private void StopButton_Click(object sender, EventArgs e)
        {
            // Нельзя останавливать если C++ уже не работает
            if (consoleApp == null || consoleApp.HasExited) return;

            // Сигналим C++: "останови последний поток"
            stopSignal.Set();

            // Ждём подтверждения
            if (confirmSignal.WaitOne(2000))
            {
                if (activeThreadsCount > 0)
                {
                    // Убираем последний пункт из списка
                    listSessions.Items.RemoveAt(listSessions.Items.Count - 1);
                    activeThreadsCount--;
                }
            }
        }

        // -------------------------------------------------------
        // Форма закрывается — корректно завершаем C++ процесс
        // -------------------------------------------------------
        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (consoleApp != null && !consoleApp.HasExited)
            {
                // Сигналим C++: "завершай работу"
                exitSignal.Set();

                // Даём 1 секунду на корректное завершение
                confirmSignal.WaitOne(1000);

                // После этого C++ процесс завершится сам
            }
        }
    }
}
