
namespace Hyperion
{
	partial class StaticModelImporter
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose( bool disposing )
		{
			if( disposing && ( components != null ) )
			{
				components.Dispose();
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.panel1 = new System.Windows.Forms.Panel();
			this.browseButton = new System.Windows.Forms.Button();
			this.fileSelectBox = new System.Windows.Forms.TextBox();
			this.importButton = new System.Windows.Forms.Button();
			this.exitButton = new System.Windows.Forms.Button();
			this.panel2 = new System.Windows.Forms.Panel();
			this.panel3 = new System.Windows.Forms.Panel();
			this.panel4 = new System.Windows.Forms.Panel();
			this.lodList = new System.Windows.Forms.ListBox();
			this.label1 = new System.Windows.Forms.Label();
			this.objList = new System.Windows.Forms.ListBox();
			this.label2 = new System.Windows.Forms.Label();
			this.panel5 = new System.Windows.Forms.Panel();
			this.label3 = new System.Windows.Forms.Label();
			this.label4 = new System.Windows.Forms.Label();
			this.label5 = new System.Windows.Forms.Label();
			this.label6 = new System.Windows.Forms.Label();
			this.materialSelect = new System.Windows.Forms.NumericUpDown();
			this.minSizeSelect = new System.Windows.Forms.NumericUpDown();
			this.panel1.SuspendLayout();
			this.panel2.SuspendLayout();
			this.panel4.SuspendLayout();
			this.panel5.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.materialSelect)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.minSizeSelect)).BeginInit();
			this.SuspendLayout();
			// 
			// panel1
			// 
			this.panel1.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(30)))), ((int)(((byte)(30)))), ((int)(((byte)(30)))));
			this.panel1.Controls.Add(this.exitButton);
			this.panel1.Controls.Add(this.importButton);
			this.panel1.Controls.Add(this.fileSelectBox);
			this.panel1.Controls.Add(this.browseButton);
			this.panel1.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.panel1.Location = new System.Drawing.Point(5, 476);
			this.panel1.Margin = new System.Windows.Forms.Padding(5);
			this.panel1.Name = "panel1";
			this.panel1.Padding = new System.Windows.Forms.Padding(5);
			this.panel1.Size = new System.Drawing.Size(790, 73);
			this.panel1.TabIndex = 1;
			// 
			// browseButton
			// 
			this.browseButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.browseButton.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(60)))), ((int)(((byte)(60)))), ((int)(((byte)(60)))));
			this.browseButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.browseButton.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.browseButton.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(200)))), ((int)(((byte)(200)))), ((int)(((byte)(200)))));
			this.browseButton.Location = new System.Drawing.Point(670, 13);
			this.browseButton.Name = "browseButton";
			this.browseButton.Size = new System.Drawing.Size(112, 23);
			this.browseButton.TabIndex = 1;
			this.browseButton.Text = "Browse";
			this.browseButton.UseVisualStyleBackColor = false;
			this.browseButton.Click += new System.EventHandler(this.browseButton_Click);
			// 
			// fileSelectBox
			// 
			this.fileSelectBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.fileSelectBox.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(60)))), ((int)(((byte)(60)))), ((int)(((byte)(60)))));
			this.fileSelectBox.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.fileSelectBox.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.fileSelectBox.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(215)))), ((int)(((byte)(215)))), ((int)(((byte)(215)))));
			this.fileSelectBox.Location = new System.Drawing.Point(12, 13);
			this.fileSelectBox.Name = "fileSelectBox";
			this.fileSelectBox.ReadOnly = true;
			this.fileSelectBox.Size = new System.Drawing.Size(652, 23);
			this.fileSelectBox.TabIndex = 5;
			// 
			// importButton
			// 
			this.importButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.importButton.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(60)))), ((int)(((byte)(60)))), ((int)(((byte)(60)))));
			this.importButton.Enabled = false;
			this.importButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.importButton.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.importButton.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(200)))), ((int)(((byte)(200)))), ((int)(((byte)(200)))));
			this.importButton.Location = new System.Drawing.Point(670, 42);
			this.importButton.Name = "importButton";
			this.importButton.Size = new System.Drawing.Size(112, 23);
			this.importButton.TabIndex = 6;
			this.importButton.Text = "Import";
			this.importButton.UseVisualStyleBackColor = false;
			this.importButton.Click += new System.EventHandler(this.importButton_Click);
			// 
			// exitButton
			// 
			this.exitButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.exitButton.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(60)))), ((int)(((byte)(60)))), ((int)(((byte)(60)))));
			this.exitButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.exitButton.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.exitButton.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(200)))), ((int)(((byte)(200)))), ((int)(((byte)(200)))));
			this.exitButton.Location = new System.Drawing.Point(12, 42);
			this.exitButton.Name = "exitButton";
			this.exitButton.Size = new System.Drawing.Size(112, 23);
			this.exitButton.TabIndex = 7;
			this.exitButton.Text = "Exit";
			this.exitButton.UseVisualStyleBackColor = false;
			this.exitButton.Click += new System.EventHandler(this.exitButton_Click);
			// 
			// panel2
			// 
			this.panel2.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(30)))), ((int)(((byte)(30)))), ((int)(((byte)(30)))));
			this.panel2.Controls.Add(this.panel5);
			this.panel2.Controls.Add(this.panel4);
			this.panel2.Controls.Add(this.panel3);
			this.panel2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel2.Location = new System.Drawing.Point(5, 5);
			this.panel2.Margin = new System.Windows.Forms.Padding(5);
			this.panel2.Name = "panel2";
			this.panel2.Size = new System.Drawing.Size(790, 471);
			this.panel2.TabIndex = 2;
			// 
			// panel3
			// 
			this.panel3.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(40)))), ((int)(((byte)(40)))), ((int)(((byte)(40)))));
			this.panel3.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.panel3.Location = new System.Drawing.Point(0, 424);
			this.panel3.Name = "panel3";
			this.panel3.Size = new System.Drawing.Size(790, 47);
			this.panel3.TabIndex = 1;
			// 
			// panel4
			// 
			this.panel4.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(40)))), ((int)(((byte)(40)))), ((int)(((byte)(40)))));
			this.panel4.Controls.Add(this.minSizeSelect);
			this.panel4.Controls.Add(this.label3);
			this.panel4.Controls.Add(this.label1);
			this.panel4.Controls.Add(this.lodList);
			this.panel4.Dock = System.Windows.Forms.DockStyle.Left;
			this.panel4.Location = new System.Drawing.Point(0, 0);
			this.panel4.Name = "panel4";
			this.panel4.Size = new System.Drawing.Size(389, 424);
			this.panel4.TabIndex = 2;
			// 
			// lodList
			// 
			this.lodList.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(30)))), ((int)(((byte)(30)))), ((int)(((byte)(30)))));
			this.lodList.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.lodList.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.lodList.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(200)))), ((int)(((byte)(200)))), ((int)(((byte)(200)))));
			this.lodList.FormattingEnabled = true;
			this.lodList.ItemHeight = 15;
			this.lodList.Items.AddRange(new object[] {
            "0"});
			this.lodList.Location = new System.Drawing.Point(12, 41);
			this.lodList.Name = "lodList";
			this.lodList.Size = new System.Drawing.Size(82, 362);
			this.lodList.TabIndex = 0;
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.label1.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(200)))), ((int)(((byte)(200)))), ((int)(((byte)(200)))));
			this.label1.Location = new System.Drawing.Point(31, 23);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(42, 15);
			this.label1.TabIndex = 1;
			this.label1.Text = "LODs";
			// 
			// objList
			// 
			this.objList.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(30)))), ((int)(((byte)(30)))), ((int)(((byte)(30)))));
			this.objList.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.objList.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.objList.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(200)))), ((int)(((byte)(200)))), ((int)(((byte)(200)))));
			this.objList.FormattingEnabled = true;
			this.objList.ItemHeight = 15;
			this.objList.Items.AddRange(new object[] {
            "0"});
			this.objList.Location = new System.Drawing.Point(6, 41);
			this.objList.Name = "objList";
			this.objList.Size = new System.Drawing.Size(82, 362);
			this.objList.TabIndex = 2;
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.label2.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(200)))), ((int)(((byte)(200)))), ((int)(((byte)(200)))));
			this.label2.Location = new System.Drawing.Point(6, 23);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(82, 15);
			this.label2.TabIndex = 3;
			this.label2.Text = "SubObjects";
			// 
			// panel5
			// 
			this.panel5.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(40)))), ((int)(((byte)(40)))), ((int)(((byte)(40)))));
			this.panel5.Controls.Add(this.materialSelect);
			this.panel5.Controls.Add(this.label6);
			this.panel5.Controls.Add(this.label5);
			this.panel5.Controls.Add(this.label4);
			this.panel5.Controls.Add(this.label2);
			this.panel5.Controls.Add(this.objList);
			this.panel5.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel5.Location = new System.Drawing.Point(389, 0);
			this.panel5.Name = "panel5";
			this.panel5.Size = new System.Drawing.Size(401, 424);
			this.panel5.TabIndex = 3;
			// 
			// label3
			// 
			this.label3.AutoSize = true;
			this.label3.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.label3.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(200)))), ((int)(((byte)(200)))), ((int)(((byte)(200)))));
			this.label3.Location = new System.Drawing.Point(157, 56);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(147, 15);
			this.label3.TabIndex = 2;
			this.label3.Text = "Minimum Screen Size";
			// 
			// label4
			// 
			this.label4.AutoSize = true;
			this.label4.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.label4.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(200)))), ((int)(((byte)(200)))), ((int)(((byte)(200)))));
			this.label4.Location = new System.Drawing.Point(172, 56);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(99, 15);
			this.label4.TabIndex = 9;
			this.label4.Text = "Vertex Count: ";
			// 
			// label5
			// 
			this.label5.AutoSize = true;
			this.label5.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.label5.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(200)))), ((int)(((byte)(200)))), ((int)(((byte)(200)))));
			this.label5.Location = new System.Drawing.Point(172, 82);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(91, 15);
			this.label5.TabIndex = 10;
			this.label5.Text = "Index Count: ";
			// 
			// label6
			// 
			this.label6.AutoSize = true;
			this.label6.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.label6.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(200)))), ((int)(((byte)(200)))), ((int)(((byte)(200)))));
			this.label6.Location = new System.Drawing.Point(172, 165);
			this.label6.Name = "label6";
			this.label6.Size = new System.Drawing.Size(90, 15);
			this.label6.TabIndex = 11;
			this.label6.Text = "Material Slot";
			// 
			// materialSelect
			// 
			this.materialSelect.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(70)))), ((int)(((byte)(70)))), ((int)(((byte)(70)))));
			this.materialSelect.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.materialSelect.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.materialSelect.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(200)))), ((int)(((byte)(200)))), ((int)(((byte)(200)))));
			this.materialSelect.Location = new System.Drawing.Point(155, 183);
			this.materialSelect.Maximum = new decimal(new int[] {
            25,
            0,
            0,
            0});
			this.materialSelect.Name = "materialSelect";
			this.materialSelect.Size = new System.Drawing.Size(120, 23);
			this.materialSelect.TabIndex = 12;
			// 
			// minSizeSelect
			// 
			this.minSizeSelect.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(70)))), ((int)(((byte)(70)))), ((int)(((byte)(70)))));
			this.minSizeSelect.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.minSizeSelect.DecimalPlaces = 1;
			this.minSizeSelect.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.minSizeSelect.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(200)))), ((int)(((byte)(200)))), ((int)(((byte)(200)))));
			this.minSizeSelect.Location = new System.Drawing.Point(171, 80);
			this.minSizeSelect.Maximum = new decimal(new int[] {
            1000000,
            0,
            0,
            0});
			this.minSizeSelect.Name = "minSizeSelect";
			this.minSizeSelect.Size = new System.Drawing.Size(120, 23);
			this.minSizeSelect.TabIndex = 13;
			// 
			// StaticModelImporter
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(50)))), ((int)(((byte)(50)))), ((int)(((byte)(50)))));
			this.ClientSize = new System.Drawing.Size(800, 554);
			this.Controls.Add(this.panel2);
			this.Controls.Add(this.panel1);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.SizableToolWindow;
			this.Name = "StaticModelImporter";
			this.Padding = new System.Windows.Forms.Padding(5);
			this.Text = "Import Static Model...";
			this.panel1.ResumeLayout(false);
			this.panel1.PerformLayout();
			this.panel2.ResumeLayout(false);
			this.panel4.ResumeLayout(false);
			this.panel4.PerformLayout();
			this.panel5.ResumeLayout(false);
			this.panel5.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.materialSelect)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.minSizeSelect)).EndInit();
			this.ResumeLayout(false);

		}

		#endregion
		private System.Windows.Forms.Panel panel1;
		private System.Windows.Forms.Button browseButton;
		public System.Windows.Forms.TextBox fileSelectBox;
		private System.Windows.Forms.Button exitButton;
		private System.Windows.Forms.Button importButton;
		private System.Windows.Forms.Panel panel2;
		private System.Windows.Forms.Panel panel3;
		private System.Windows.Forms.Panel panel4;
		private System.Windows.Forms.ListBox lodList;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.ListBox objList;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Panel panel5;
		private System.Windows.Forms.NumericUpDown materialSelect;
		private System.Windows.Forms.Label label6;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.NumericUpDown minSizeSelect;
		private System.Windows.Forms.Label label3;
	}
}