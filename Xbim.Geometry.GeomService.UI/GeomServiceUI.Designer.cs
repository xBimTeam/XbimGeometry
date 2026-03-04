namespace Xbim.Geometry.GeomService.UI
{
	partial class GeomServiceUI
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if (disposing && (components != null))
			{
				components.Dispose();
			}
			base.Dispose(disposing);
		}

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            txtSources = new TextBox();
            label1 = new Label();
            cmdConvertGeometry = new Button();
            btnSearchGlob = new Button();
            listBoxLog = new ListBox();
            grpProgress = new GroupBox();
            actionLog = new ListBox();
            progFiles = new ProgressBar();
            label3 = new Label();
            label2 = new Label();
            progSingleFile = new ProgressBar();
            button1 = new Button();
            cmdCancelConvertGeometry = new Button();
            groupBox1 = new GroupBox();
            label8 = new Label();
            nudMemoryLimit = new NumericUpDown();
            label9 = new Label();
            label7 = new Label();
            cmbLoggingLevel = new ComboBox();
            cmdMoveDown = new Button();
            cmdMoveUp = new Button();
            lstSequence = new ListView();
            columnHeader1 = new ColumnHeader();
            label6 = new Label();
            label5 = new Label();
            nudTimeoutMinutes = new NumericUpDown();
            label4 = new Label();
            button2 = new Button();
            cmdRequestDump = new Button();
            chkSkipMeshed = new CheckBox();
            groupBox2 = new GroupBox();
            cmdEnumerateFiles = new Button();
            grpProgress.SuspendLayout();
            groupBox1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)nudMemoryLimit).BeginInit();
            ((System.ComponentModel.ISupportInitialize)nudTimeoutMinutes).BeginInit();
            groupBox2.SuspendLayout();
            SuspendLayout();
            // 
            // txtSources
            // 
            txtSources.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
            txtSources.Location = new Point(13, 27);
            txtSources.Margin = new Padding(2);
            txtSources.Name = "txtSources";
            txtSources.ScrollBars = ScrollBars.Vertical;
            txtSources.Size = new Size(820, 23);
            txtSources.TabIndex = 0;
            // 
            // label1
            // 
            label1.AutoSize = true;
            label1.Location = new Point(8, 6);
            label1.Margin = new Padding(2, 0, 2, 0);
            label1.Name = "label1";
            label1.Size = new Size(48, 15);
            label1.TabIndex = 1;
            label1.Text = "Sources";
            // 
            // cmdConvertGeometry
            // 
            cmdConvertGeometry.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
            cmdConvertGeometry.Font = new Font("Microsoft Sans Serif", 8.25F, FontStyle.Bold, GraphicsUnit.Point, 0);
            cmdConvertGeometry.Location = new Point(5, 21);
            cmdConvertGeometry.Margin = new Padding(2);
            cmdConvertGeometry.Name = "cmdConvertGeometry";
            cmdConvertGeometry.Size = new Size(280, 35);
            cmdConvertGeometry.TabIndex = 2;
            cmdConvertGeometry.Text = "Evaluate Geometry";
            cmdConvertGeometry.UseVisualStyleBackColor = true;
            cmdConvertGeometry.Click += cmdConvertGeometry_Click;
            // 
            // btnSearchGlob
            // 
            btnSearchGlob.Anchor = AnchorStyles.Top | AnchorStyles.Right;
            btnSearchGlob.Location = new Point(839, 25);
            btnSearchGlob.Margin = new Padding(4, 3, 4, 3);
            btnSearchGlob.Name = "btnSearchGlob";
            btnSearchGlob.Size = new Size(88, 25);
            btnSearchGlob.TabIndex = 4;
            btnSearchGlob.Text = "...";
            btnSearchGlob.UseVisualStyleBackColor = true;
            btnSearchGlob.Click += btnSearchGlob_Click;
            // 
            // listBoxLog
            // 
            listBoxLog.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
            listBoxLog.FormattingEnabled = true;
            listBoxLog.ItemHeight = 15;
            listBoxLog.Location = new Point(14, 55);
            listBoxLog.Margin = new Padding(4, 3, 4, 3);
            listBoxLog.Name = "listBoxLog";
            listBoxLog.ScrollAlwaysVisible = true;
            listBoxLog.Size = new Size(912, 274);
            listBoxLog.TabIndex = 5;
            listBoxLog.KeyUp += listBoxLog_KeyUp;
            listBoxLog.MouseDoubleClick += listBoxLog_MouseDoubleClick;
            // 
            // grpProgress
            // 
            grpProgress.Anchor = AnchorStyles.Bottom | AnchorStyles.Left;
            grpProgress.Controls.Add(actionLog);
            grpProgress.Controls.Add(progFiles);
            grpProgress.Controls.Add(label3);
            grpProgress.Controls.Add(label2);
            grpProgress.Controls.Add(progSingleFile);
            grpProgress.Location = new Point(14, 343);
            grpProgress.Margin = new Padding(4, 3, 4, 3);
            grpProgress.Name = "grpProgress";
            grpProgress.Padding = new Padding(4, 3, 4, 3);
            grpProgress.Size = new Size(290, 243);
            grpProgress.TabIndex = 6;
            grpProgress.TabStop = false;
            grpProgress.Text = "Progress";
            // 
            // actionLog
            // 
            actionLog.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
            actionLog.FormattingEnabled = true;
            actionLog.ItemHeight = 15;
            actionLog.Location = new Point(10, 67);
            actionLog.Margin = new Padding(4, 3, 4, 3);
            actionLog.Name = "actionLog";
            actionLog.ScrollAlwaysVisible = true;
            actionLog.Size = new Size(272, 154);
            actionLog.TabIndex = 6;
            // 
            // progFiles
            // 
            progFiles.Location = new Point(72, 22);
            progFiles.Margin = new Padding(4, 3, 4, 3);
            progFiles.Name = "progFiles";
            progFiles.Size = new Size(211, 16);
            progFiles.TabIndex = 3;
            // 
            // label3
            // 
            label3.AutoSize = true;
            label3.Location = new Point(7, 45);
            label3.Margin = new Padding(4, 0, 4, 0);
            label3.Name = "label3";
            label3.Size = new Size(25, 15);
            label3.TabIndex = 2;
            label3.Text = "File";
            // 
            // label2
            // 
            label2.AutoSize = true;
            label2.Location = new Point(7, 22);
            label2.Margin = new Padding(4, 0, 4, 0);
            label2.Name = "label2";
            label2.Size = new Size(30, 15);
            label2.TabIndex = 1;
            label2.Text = "Files";
            // 
            // progSingleFile
            // 
            progSingleFile.Location = new Point(72, 45);
            progSingleFile.Margin = new Padding(4, 3, 4, 3);
            progSingleFile.Name = "progSingleFile";
            progSingleFile.Size = new Size(211, 16);
            progSingleFile.TabIndex = 0;
            // 
            // button1
            // 
            button1.Anchor = AnchorStyles.Bottom | AnchorStyles.Right;
            button1.Location = new Point(636, 433);
            button1.Margin = new Padding(2);
            button1.Name = "button1";
            button1.Size = new Size(140, 25);
            button1.TabIndex = 7;
            button1.Text = "Open log";
            button1.UseVisualStyleBackColor = true;
            button1.Click += button1_Click;
            // 
            // cmdCancelConvertGeometry
            // 
            cmdCancelConvertGeometry.Anchor = AnchorStyles.Bottom | AnchorStyles.Right;
            cmdCancelConvertGeometry.Enabled = false;
            cmdCancelConvertGeometry.Font = new Font("Microsoft Sans Serif", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
            cmdCancelConvertGeometry.Location = new Point(636, 560);
            cmdCancelConvertGeometry.Margin = new Padding(2);
            cmdCancelConvertGeometry.Name = "cmdCancelConvertGeometry";
            cmdCancelConvertGeometry.Size = new Size(140, 25);
            cmdCancelConvertGeometry.TabIndex = 8;
            cmdCancelConvertGeometry.Text = "Cancel";
            cmdCancelConvertGeometry.UseVisualStyleBackColor = true;
            cmdCancelConvertGeometry.Click += cmdCancelConvertGeometry_Click;
            // 
            // groupBox1
            // 
            groupBox1.Anchor = AnchorStyles.Bottom | AnchorStyles.Left;
            groupBox1.Controls.Add(label8);
            groupBox1.Controls.Add(nudMemoryLimit);
            groupBox1.Controls.Add(label9);
            groupBox1.Controls.Add(label7);
            groupBox1.Controls.Add(cmbLoggingLevel);
            groupBox1.Controls.Add(cmdMoveDown);
            groupBox1.Controls.Add(cmdMoveUp);
            groupBox1.Controls.Add(lstSequence);
            groupBox1.Controls.Add(label6);
            groupBox1.Controls.Add(label5);
            groupBox1.Controls.Add(nudTimeoutMinutes);
            groupBox1.Controls.Add(label4);
            groupBox1.Location = new Point(321, 343);
            groupBox1.Margin = new Padding(4, 3, 4, 3);
            groupBox1.Name = "groupBox1";
            groupBox1.Padding = new Padding(4, 3, 4, 3);
            groupBox1.Size = new Size(290, 243);
            groupBox1.TabIndex = 9;
            groupBox1.TabStop = false;
            groupBox1.Text = "Settings";
            // 
            // label8
            // 
            label8.AutoSize = true;
            label8.Location = new Point(223, 47);
            label8.Margin = new Padding(4, 0, 4, 0);
            label8.Name = "label8";
            label8.Size = new Size(25, 15);
            label8.TabIndex = 11;
            label8.Text = "Mb";
            // 
            // nudMemoryLimit
            // 
            nudMemoryLimit.Location = new Point(140, 44);
            nudMemoryLimit.Margin = new Padding(4, 3, 4, 3);
            nudMemoryLimit.Maximum = new decimal(new int[] { 8192, 0, 0, 0 });
            nudMemoryLimit.Minimum = new decimal(new int[] { 128, 0, 0, 0 });
            nudMemoryLimit.Name = "nudMemoryLimit";
            nudMemoryLimit.Size = new Size(78, 23);
            nudMemoryLimit.TabIndex = 10;
            nudMemoryLimit.TextAlign = HorizontalAlignment.Right;
            nudMemoryLimit.Value = new decimal(new int[] { 4096, 0, 0, 0 });
            // 
            // label9
            // 
            label9.AutoSize = true;
            label9.Location = new Point(7, 47);
            label9.Margin = new Padding(4, 0, 4, 0);
            label9.Name = "label9";
            label9.Size = new Size(82, 15);
            label9.TabIndex = 9;
            label9.Text = "Memory limit:";
            // 
            // label7
            // 
            label7.AutoSize = true;
            label7.Location = new Point(8, 76);
            label7.Margin = new Padding(4, 0, 4, 0);
            label7.Name = "label7";
            label7.Size = new Size(57, 15);
            label7.TabIndex = 8;
            label7.Text = "Log level:";
            // 
            // cmbLoggingLevel
            // 
            cmbLoggingLevel.DropDownStyle = ComboBoxStyle.DropDownList;
            cmbLoggingLevel.FormattingEnabled = true;
            cmbLoggingLevel.Items.AddRange(new object[] { "Trace", "Debug", "Information", "Warning", "Error", "Critical" });
            cmbLoggingLevel.Location = new Point(140, 73);
            cmbLoggingLevel.Margin = new Padding(4, 3, 4, 3);
            cmbLoggingLevel.Name = "cmbLoggingLevel";
            cmbLoggingLevel.Size = new Size(132, 23);
            cmbLoggingLevel.TabIndex = 7;
            // 
            // cmdMoveDown
            // 
            cmdMoveDown.Font = new Font("Microsoft Sans Serif", 15.75F, FontStyle.Bold, GraphicsUnit.Point, 0);
            cmdMoveDown.ForeColor = SystemColors.ActiveCaptionText;
            cmdMoveDown.Location = new Point(100, 194);
            cmdMoveDown.Margin = new Padding(4, 3, 4, 3);
            cmdMoveDown.Name = "cmdMoveDown";
            cmdMoveDown.Size = new Size(33, 39);
            cmdMoveDown.TabIndex = 6;
            cmdMoveDown.Text = "⇩";
            cmdMoveDown.UseVisualStyleBackColor = true;
            cmdMoveDown.Click += cmdMoveDown_Click;
            // 
            // cmdMoveUp
            // 
            cmdMoveUp.Font = new Font("Microsoft Sans Serif", 15.75F, FontStyle.Bold, GraphicsUnit.Point, 0);
            cmdMoveUp.ForeColor = SystemColors.ActiveCaptionText;
            cmdMoveUp.Location = new Point(100, 149);
            cmdMoveUp.Margin = new Padding(4, 3, 4, 3);
            cmdMoveUp.Name = "cmdMoveUp";
            cmdMoveUp.Size = new Size(33, 39);
            cmdMoveUp.TabIndex = 5;
            cmdMoveUp.Text = "⇧";
            cmdMoveUp.UseVisualStyleBackColor = true;
            cmdMoveUp.Click += cmdMoveUp_Click;
            // 
            // lstSequence
            // 
            lstSequence.CheckBoxes = true;
            lstSequence.Columns.AddRange(new ColumnHeader[] { columnHeader1 });
            lstSequence.FullRowSelect = true;
            lstSequence.GridLines = true;
            lstSequence.Location = new Point(141, 102);
            lstSequence.Margin = new Padding(4, 3, 4, 3);
            lstSequence.MultiSelect = false;
            lstSequence.Name = "lstSequence";
            lstSequence.Size = new Size(132, 131);
            lstSequence.TabIndex = 4;
            lstSequence.UseCompatibleStateImageBehavior = false;
            lstSequence.View = View.Details;
            // 
            // columnHeader1
            // 
            columnHeader1.Text = "Mode";
            columnHeader1.Width = 110;
            // 
            // label6
            // 
            label6.AutoSize = true;
            label6.Location = new Point(7, 106);
            label6.Margin = new Padding(4, 0, 4, 0);
            label6.Name = "label6";
            label6.Size = new Size(105, 15);
            label6.TabIndex = 3;
            label6.Text = "Modes to attempt:";
            // 
            // label5
            // 
            label5.AutoSize = true;
            label5.Location = new Point(223, 18);
            label5.Margin = new Padding(4, 0, 4, 0);
            label5.Name = "label5";
            label5.Size = new Size(50, 15);
            label5.TabIndex = 2;
            label5.Text = "minutes";
            // 
            // nudTimeoutMinutes
            // 
            nudTimeoutMinutes.Location = new Point(140, 15);
            nudTimeoutMinutes.Margin = new Padding(4, 3, 4, 3);
            nudTimeoutMinutes.Maximum = new decimal(new int[] { 120, 0, 0, 0 });
            nudTimeoutMinutes.Minimum = new decimal(new int[] { 5, 0, 0, 0 });
            nudTimeoutMinutes.Name = "nudTimeoutMinutes";
            nudTimeoutMinutes.Size = new Size(78, 23);
            nudTimeoutMinutes.TabIndex = 1;
            nudTimeoutMinutes.TextAlign = HorizontalAlignment.Right;
            nudTimeoutMinutes.Value = new decimal(new int[] { 10, 0, 0, 0 });
            // 
            // label4
            // 
            label4.AutoSize = true;
            label4.Location = new Point(7, 18);
            label4.Margin = new Padding(4, 0, 4, 0);
            label4.Name = "label4";
            label4.Size = new Size(124, 15);
            label4.TabIndex = 0;
            label4.Text = "Single model timeout:";
            // 
            // button2
            // 
            button2.Anchor = AnchorStyles.Bottom | AnchorStyles.Right;
            button2.Location = new Point(786, 433);
            button2.Margin = new Padding(2);
            button2.Name = "button2";
            button2.Size = new Size(140, 25);
            button2.TabIndex = 10;
            button2.Text = "Delete geom logs";
            button2.UseVisualStyleBackColor = true;
            button2.Click += button2_Click;
            // 
            // cmdRequestDump
            // 
            cmdRequestDump.Anchor = AnchorStyles.Bottom | AnchorStyles.Right;
            cmdRequestDump.Enabled = false;
            cmdRequestDump.Font = new Font("Microsoft Sans Serif", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
            cmdRequestDump.Location = new Point(786, 560);
            cmdRequestDump.Margin = new Padding(2);
            cmdRequestDump.Name = "cmdRequestDump";
            cmdRequestDump.Size = new Size(140, 25);
            cmdRequestDump.TabIndex = 11;
            cmdRequestDump.Text = "Dump Stack";
            cmdRequestDump.UseVisualStyleBackColor = true;
            cmdRequestDump.Click += button3_Click;
            // 
            // chkSkipMeshed
            // 
            chkSkipMeshed.AutoSize = true;
            chkSkipMeshed.Location = new Point(6, 65);
            chkSkipMeshed.Name = "chkSkipMeshed";
            chkSkipMeshed.Size = new Size(158, 19);
            chkSkipMeshed.TabIndex = 12;
            chkSkipMeshed.Text = "skip previously evaluated";
            chkSkipMeshed.UseVisualStyleBackColor = true;
            // 
            // groupBox2
            // 
            groupBox2.Anchor = AnchorStyles.Bottom | AnchorStyles.Right;
            groupBox2.Controls.Add(cmdEnumerateFiles);
            groupBox2.Controls.Add(cmdConvertGeometry);
            groupBox2.Controls.Add(chkSkipMeshed);
            groupBox2.Location = new Point(636, 463);
            groupBox2.Name = "groupBox2";
            groupBox2.Size = new Size(290, 92);
            groupBox2.TabIndex = 13;
            groupBox2.TabStop = false;
            groupBox2.Text = "Execute";
            // 
            // cmdEnumerateFiles
            // 
            cmdEnumerateFiles.Location = new Point(169, 59);
            cmdEnumerateFiles.Margin = new Padding(2);
            cmdEnumerateFiles.Name = "cmdEnumerateFiles";
            cmdEnumerateFiles.Size = new Size(116, 28);
            cmdEnumerateFiles.TabIndex = 13;
            cmdEnumerateFiles.Text = "Enumerate files";
            cmdEnumerateFiles.UseVisualStyleBackColor = true;
            cmdEnumerateFiles.Click += cmdEnumerateFiles_Click;
            // 
            // GeomServiceUI
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(938, 599);
            Controls.Add(groupBox2);
            Controls.Add(cmdRequestDump);
            Controls.Add(button2);
            Controls.Add(groupBox1);
            Controls.Add(button1);
            Controls.Add(cmdCancelConvertGeometry);
            Controls.Add(grpProgress);
            Controls.Add(listBoxLog);
            Controls.Add(btnSearchGlob);
            Controls.Add(label1);
            Controls.Add(txtSources);
            Margin = new Padding(2);
            MinimumSize = new Size(954, 513);
            Name = "GeomServiceUI";
            Text = "GeometryTester";
            grpProgress.ResumeLayout(false);
            grpProgress.PerformLayout();
            groupBox1.ResumeLayout(false);
            groupBox1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)nudMemoryLimit).EndInit();
            ((System.ComponentModel.ISupportInitialize)nudTimeoutMinutes).EndInit();
            groupBox2.ResumeLayout(false);
            groupBox2.PerformLayout();
            ResumeLayout(false);
            PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox txtSources;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Button cmdConvertGeometry;
		private System.Windows.Forms.Button btnSearchGlob;
		private System.Windows.Forms.ListBox listBoxLog;
		private System.Windows.Forms.GroupBox grpProgress;
		private System.Windows.Forms.ProgressBar progFiles;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.ProgressBar progSingleFile;
		private System.Windows.Forms.Button button1;
		private System.Windows.Forms.Button cmdCancelConvertGeometry;
		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.NumericUpDown nudTimeoutMinutes;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.ListBox actionLog;
		private System.Windows.Forms.Button button2;
		private System.Windows.Forms.ListView lstSequence;
		private System.Windows.Forms.Label label6;
		private System.Windows.Forms.ColumnHeader columnHeader1;
		private System.Windows.Forms.Button cmdMoveDown;
		private System.Windows.Forms.Button cmdMoveUp;
		private System.Windows.Forms.ComboBox cmbLoggingLevel;
		private System.Windows.Forms.Label label7;
        private Button cmdRequestDump;
        private Label label8;
        private NumericUpDown nudMemoryLimit;
        private Label label9;
        private CheckBox chkSkipMeshed;
        private GroupBox groupBox2;
        private Button cmdEnumerateFiles;
    }
}