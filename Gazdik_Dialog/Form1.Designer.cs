// ============================================================
// Form1.Designer.cs — описание внешнего вида формы
//
// Этот файл генерируется Visual Studio автоматически
// когда ты рисуешь форму в дизайнере (перетаскиваешь кнопки).
// Руками его лучше не трогать — VS перепишет изменения.
//
// Здесь описано:
//   - Какие элементы есть на форме (кнопки, список, поле)
//   - Где они расположены (Location)
//   - Какого размера (Size)
//   - Какой текст на них (Text)
//   - К каким обработчикам подключены (Click += ...)
// ============================================================

namespace Gazdik_Dialog
{
    partial class Form1
    {
        private System.ComponentModel.IContainer components = null;

        // Вызывается при закрытии формы — очищаем ресурсы
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        private void InitializeComponent()
        {
            // Создаём все элементы управления
            StartButton = new Button();
            StopButton = new Button();
            numberThreads = new NumericUpDown();
            listSessions = new ListBox();
            label1 = new Label();

            ((System.ComponentModel.ISupportInitialize)numberThreads).BeginInit();
            SuspendLayout();

            // ---- Кнопка Start ----
            StartButton.Location = new Point(524, 83);
            StartButton.Margin = new Padding(4, 5, 4, 5);
            StartButton.Name = "StartButton";
            StartButton.Size = new Size(243, 73);
            StartButton.TabIndex = 0;
            StartButton.Text = "Start";
            StartButton.UseVisualStyleBackColor = true;
            StartButton.Click += StartButton_Click;  // привязка к обработчику

            // ---- Кнопка Stop ----
            StopButton.Location = new Point(802, 83);
            StopButton.Margin = new Padding(4, 5, 4, 5);
            StopButton.Name = "StopButton";
            StopButton.Size = new Size(224, 73);
            StopButton.TabIndex = 1;
            StopButton.Text = "Stop";
            StopButton.UseVisualStyleBackColor = true;
            StopButton.Click += StopButton_Click;

            // ---- Числовой счётчик (сколько потоков создать) ----
            numberThreads.Location = new Point(855, 47);
            numberThreads.Margin = new Padding(4, 5, 4, 5);
            numberThreads.Maximum = new decimal(new int[] { 1000, 0, 0, 0 });  // максимум 1000
            numberThreads.Minimum = new decimal(new int[] { 1, 0, 0, 0 });     // минимум 1
            numberThreads.Name = "numberThreads";
            numberThreads.Size = new Size(171, 31);
            numberThreads.TabIndex = 2;
            numberThreads.Value = new decimal(new int[] { 1, 0, 0, 0 });       // начальное значение 1

            // ---- Список активных потоков ----
            listSessions.FormattingEnabled = true;
            listSessions.Location = new Point(17, 38);
            listSessions.Margin = new Padding(4, 5, 4, 5);
            listSessions.Name = "listSessions";
            listSessions.Size = new Size(475, 654);
            listSessions.TabIndex = 3;

            // ---- Подпись над счётчиком ----
            label1.AutoSize = true;
            label1.Font = new Font("Segoe UI", 12F);
            label1.Location = new Point(524, 43);
            label1.Margin = new Padding(4, 0, 4, 0);
            label1.Name = "label1";
            label1.Size = new Size(323, 32);
            label1.TabIndex = 4;
            label1.Text = "Количество новых потоков:";

            // ---- Настройки самой формы ----
            AutoScaleDimensions = new SizeF(10F, 25F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(1143, 750);
            Controls.Add(label1);
            Controls.Add(listSessions);
            Controls.Add(numberThreads);
            Controls.Add(StopButton);
            Controls.Add(StartButton);
            Margin = new Padding(4, 5, 4, 5);
            Name = "Form1";
            Text = "Gazdik — Лабораторная работа 1";  // заголовок окна
            FormClosing += Form1_FormClosing;

            ((System.ComponentModel.ISupportInitialize)numberThreads).EndInit();
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        // Объявления полей — Visual Studio добавляет сюда автоматически
        private Button StartButton;
        private Button StopButton;
        private NumericUpDown numberThreads;
        private ListBox listSessions;
        private Label label1;
    }
}
