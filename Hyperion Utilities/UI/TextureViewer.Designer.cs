namespace Hyperion
{
	partial class TextureViewer
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
			this.lodList = new System.Windows.Forms.ListBox();
			this.label1 = new System.Windows.Forms.Label();
			this.panel2 = new System.Windows.Forms.Panel();
			this.previewPanel = new System.Windows.Forms.Panel();
			this.imagePanel = new System.Windows.Forms.Panel();
			this.label2 = new System.Windows.Forms.Label();
			this.panel3 = new System.Windows.Forms.Panel();
			this.cursorLabel = new System.Windows.Forms.Label();
			this.formatLabel = new System.Windows.Forms.Label();
			this.lodSizeLabel = new System.Windows.Forms.Label();
			this.texSizeLabel = new System.Windows.Forms.Label();
			this.clearButton = new System.Windows.Forms.Button();
			this.browseButton = new System.Windows.Forms.Button();
			this.fileBox = new System.Windows.Forms.TextBox();
			this.panel1.SuspendLayout();
			this.panel2.SuspendLayout();
			this.previewPanel.SuspendLayout();
			this.panel3.SuspendLayout();
			this.SuspendLayout();
			// 
			// panel1
			// 
			this.panel1.Controls.Add(this.lodList);
			this.panel1.Controls.Add(this.label1);
			this.panel1.Dock = System.Windows.Forms.DockStyle.Left;
			this.panel1.Location = new System.Drawing.Point(0, 0);
			this.panel1.Name = "panel1";
			this.panel1.Padding = new System.Windows.Forms.Padding(8);
			this.panel1.Size = new System.Drawing.Size(134, 543);
			this.panel1.TabIndex = 0;
			this.panel1.MouseMove += new System.Windows.Forms.MouseEventHandler(this.imagePanel_MouseMove);
			// 
			// lodList
			// 
			this.lodList.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(30)))), ((int)(((byte)(30)))), ((int)(((byte)(30)))));
			this.lodList.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.lodList.Dock = System.Windows.Forms.DockStyle.Fill;
			this.lodList.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.lodList.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(210)))), ((int)(((byte)(210)))), ((int)(((byte)(210)))));
			this.lodList.FormattingEnabled = true;
			this.lodList.ItemHeight = 15;
			this.lodList.Location = new System.Drawing.Point(8, 37);
			this.lodList.Name = "lodList";
			this.lodList.Size = new System.Drawing.Size(118, 498);
			this.lodList.TabIndex = 2;
			this.lodList.SelectedIndexChanged += new System.EventHandler(this.lodList_SelectedIndexChanged);
			this.lodList.MouseMove += new System.Windows.Forms.MouseEventHandler(this.imagePanel_MouseMove);
			// 
			// label1
			// 
			this.label1.Dock = System.Windows.Forms.DockStyle.Top;
			this.label1.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.label1.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(210)))), ((int)(((byte)(210)))), ((int)(((byte)(210)))));
			this.label1.Location = new System.Drawing.Point(8, 8);
			this.label1.Name = "label1";
			this.label1.Padding = new System.Windows.Forms.Padding(0, 0, 0, 8);
			this.label1.Size = new System.Drawing.Size(118, 29);
			this.label1.TabIndex = 0;
			this.label1.Text = "LOD(s)";
			this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			this.label1.MouseMove += new System.Windows.Forms.MouseEventHandler(this.imagePanel_MouseMove);
			// 
			// panel2
			// 
			this.panel2.Controls.Add(this.previewPanel);
			this.panel2.Controls.Add(this.label2);
			this.panel2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel2.Location = new System.Drawing.Point(134, 0);
			this.panel2.Name = "panel2";
			this.panel2.Padding = new System.Windows.Forms.Padding(2, 8, 8, 8);
			this.panel2.Size = new System.Drawing.Size(816, 543);
			this.panel2.TabIndex = 1;
			this.panel2.MouseMove += new System.Windows.Forms.MouseEventHandler(this.imagePanel_MouseMove);
			// 
			// previewPanel
			// 
			this.previewPanel.AutoScroll = true;
			this.previewPanel.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(30)))), ((int)(((byte)(30)))), ((int)(((byte)(30)))));
			this.previewPanel.Controls.Add(this.imagePanel);
			this.previewPanel.Dock = System.Windows.Forms.DockStyle.Fill;
			this.previewPanel.Location = new System.Drawing.Point(2, 37);
			this.previewPanel.Name = "previewPanel";
			this.previewPanel.Size = new System.Drawing.Size(806, 498);
			this.previewPanel.TabIndex = 2;
			this.previewPanel.MouseDown += new System.Windows.Forms.MouseEventHandler(this.imagePanel_MouseDown);
			this.previewPanel.MouseMove += new System.Windows.Forms.MouseEventHandler(this.imagePanel_MouseMove);
			this.previewPanel.MouseUp += new System.Windows.Forms.MouseEventHandler(this.imagePanel_MouseUp);
			// 
			// imagePanel
			// 
			this.imagePanel.Anchor = System.Windows.Forms.AnchorStyles.None;
			this.imagePanel.Location = new System.Drawing.Point(282, 183);
			this.imagePanel.Name = "imagePanel";
			this.imagePanel.Size = new System.Drawing.Size(200, 100);
			this.imagePanel.TabIndex = 0;
			this.imagePanel.Paint += new System.Windows.Forms.PaintEventHandler(this.imagePanel_Paint);
			this.imagePanel.MouseDown += new System.Windows.Forms.MouseEventHandler(this.imagePanel_MouseDown);
			this.imagePanel.MouseMove += new System.Windows.Forms.MouseEventHandler(this.imagePanel_MouseMove);
			this.imagePanel.MouseUp += new System.Windows.Forms.MouseEventHandler(this.imagePanel_MouseUp);
			// 
			// label2
			// 
			this.label2.Dock = System.Windows.Forms.DockStyle.Top;
			this.label2.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.label2.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(210)))), ((int)(((byte)(210)))), ((int)(((byte)(210)))));
			this.label2.Location = new System.Drawing.Point(2, 8);
			this.label2.Name = "label2";
			this.label2.Padding = new System.Windows.Forms.Padding(0, 0, 0, 8);
			this.label2.Size = new System.Drawing.Size(806, 29);
			this.label2.TabIndex = 1;
			this.label2.Text = "Texture Preview";
			this.label2.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			this.label2.MouseMove += new System.Windows.Forms.MouseEventHandler(this.imagePanel_MouseMove);
			// 
			// panel3
			// 
			this.panel3.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(30)))), ((int)(((byte)(30)))), ((int)(((byte)(30)))));
			this.panel3.Controls.Add(this.cursorLabel);
			this.panel3.Controls.Add(this.formatLabel);
			this.panel3.Controls.Add(this.lodSizeLabel);
			this.panel3.Controls.Add(this.texSizeLabel);
			this.panel3.Controls.Add(this.clearButton);
			this.panel3.Controls.Add(this.browseButton);
			this.panel3.Controls.Add(this.fileBox);
			this.panel3.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.panel3.Location = new System.Drawing.Point(0, 543);
			this.panel3.Name = "panel3";
			this.panel3.Size = new System.Drawing.Size(950, 86);
			this.panel3.TabIndex = 2;
			this.panel3.MouseMove += new System.Windows.Forms.MouseEventHandler(this.imagePanel_MouseMove);
			// 
			// cursorLabel
			// 
			this.cursorLabel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.cursorLabel.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.cursorLabel.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(210)))), ((int)(((byte)(210)))), ((int)(((byte)(210)))));
			this.cursorLabel.Location = new System.Drawing.Point(633, 3);
			this.cursorLabel.Name = "cursorLabel";
			this.cursorLabel.Size = new System.Drawing.Size(314, 45);
			this.cursorLabel.TabIndex = 5;
			this.cursorLabel.Text = "Cursor: 00000x00000 [000,000,000,0000]";
			this.cursorLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			this.cursorLabel.MouseMove += new System.Windows.Forms.MouseEventHandler(this.imagePanel_MouseMove);
			// 
			// formatLabel
			// 
			this.formatLabel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.formatLabel.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.formatLabel.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(210)))), ((int)(((byte)(210)))), ((int)(((byte)(210)))));
			this.formatLabel.Location = new System.Drawing.Point(426, 3);
			this.formatLabel.Name = "formatLabel";
			this.formatLabel.Size = new System.Drawing.Size(201, 45);
			this.formatLabel.TabIndex = 4;
			this.formatLabel.Text = "Format: RGBA_8BIT_UNORM_SRGB";
			this.formatLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			this.formatLabel.MouseMove += new System.Windows.Forms.MouseEventHandler(this.imagePanel_MouseMove);
			// 
			// lodSizeLabel
			// 
			this.lodSizeLabel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.lodSizeLabel.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.lodSizeLabel.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(210)))), ((int)(((byte)(210)))), ((int)(((byte)(210)))));
			this.lodSizeLabel.Location = new System.Drawing.Point(219, 3);
			this.lodSizeLabel.Name = "lodSizeLabel";
			this.lodSizeLabel.Size = new System.Drawing.Size(201, 45);
			this.lodSizeLabel.TabIndex = 3;
			this.lodSizeLabel.Text = "LOD Size: 00000x00000 00000000kb";
			this.lodSizeLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			this.lodSizeLabel.MouseMove += new System.Windows.Forms.MouseEventHandler(this.imagePanel_MouseMove);
			// 
			// texSizeLabel
			// 
			this.texSizeLabel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.texSizeLabel.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.texSizeLabel.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(210)))), ((int)(((byte)(210)))), ((int)(((byte)(210)))));
			this.texSizeLabel.Location = new System.Drawing.Point(12, 3);
			this.texSizeLabel.Name = "texSizeLabel";
			this.texSizeLabel.Size = new System.Drawing.Size(201, 47);
			this.texSizeLabel.TabIndex = 2;
			this.texSizeLabel.Text = "Full Size: 00000x00000 0000000000kb";
			this.texSizeLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			this.texSizeLabel.MouseMove += new System.Windows.Forms.MouseEventHandler(this.imagePanel_MouseMove);
			// 
			// clearButton
			// 
			this.clearButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.clearButton.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(60)))), ((int)(((byte)(60)))), ((int)(((byte)(60)))));
			this.clearButton.FlatAppearance.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(100)))), ((int)(((byte)(100)))), ((int)(((byte)(100)))));
			this.clearButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.clearButton.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.clearButton.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(210)))), ((int)(((byte)(210)))), ((int)(((byte)(210)))));
			this.clearButton.Location = new System.Drawing.Point(863, 51);
			this.clearButton.Name = "clearButton";
			this.clearButton.Size = new System.Drawing.Size(75, 23);
			this.clearButton.TabIndex = 2;
			this.clearButton.Text = "Clear";
			this.clearButton.UseVisualStyleBackColor = false;
			this.clearButton.Click += new System.EventHandler(this.clearButton_Click);
			this.clearButton.MouseMove += new System.Windows.Forms.MouseEventHandler(this.imagePanel_MouseMove);
			// 
			// browseButton
			// 
			this.browseButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.browseButton.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(60)))), ((int)(((byte)(60)))), ((int)(((byte)(60)))));
			this.browseButton.FlatAppearance.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(100)))), ((int)(((byte)(100)))), ((int)(((byte)(100)))));
			this.browseButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.browseButton.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.browseButton.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(210)))), ((int)(((byte)(210)))), ((int)(((byte)(210)))));
			this.browseButton.Location = new System.Drawing.Point(782, 51);
			this.browseButton.Name = "browseButton";
			this.browseButton.Size = new System.Drawing.Size(75, 23);
			this.browseButton.TabIndex = 0;
			this.browseButton.Text = "Browse";
			this.browseButton.UseVisualStyleBackColor = false;
			this.browseButton.Click += new System.EventHandler(this.browseButton_Click);
			this.browseButton.MouseMove += new System.Windows.Forms.MouseEventHandler(this.imagePanel_MouseMove);
			// 
			// fileBox
			// 
			this.fileBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.fileBox.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(60)))), ((int)(((byte)(60)))), ((int)(((byte)(60)))));
			this.fileBox.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.fileBox.Enabled = false;
			this.fileBox.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.fileBox.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(210)))), ((int)(((byte)(210)))), ((int)(((byte)(210)))));
			this.fileBox.Location = new System.Drawing.Point(11, 53);
			this.fileBox.Name = "fileBox";
			this.fileBox.Size = new System.Drawing.Size(764, 21);
			this.fileBox.TabIndex = 0;
			this.fileBox.MouseMove += new System.Windows.Forms.MouseEventHandler(this.imagePanel_MouseMove);
			// 
			// TextureViewer
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(50)))), ((int)(((byte)(50)))), ((int)(((byte)(50)))));
			this.ClientSize = new System.Drawing.Size(950, 629);
			this.Controls.Add(this.panel2);
			this.Controls.Add(this.panel1);
			this.Controls.Add(this.panel3);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.SizableToolWindow;
			this.Name = "TextureViewer";
			this.Text = "Texture Viewer";
			this.MouseMove += new System.Windows.Forms.MouseEventHandler(this.imagePanel_MouseMove);
			this.Resize += new System.EventHandler(this.TextureViewer_Resize);
			this.panel1.ResumeLayout(false);
			this.panel2.ResumeLayout(false);
			this.previewPanel.ResumeLayout(false);
			this.panel3.ResumeLayout(false);
			this.panel3.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.Panel panel1;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Panel panel2;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Panel panel3;
		private System.Windows.Forms.TextBox fileBox;
		private System.Windows.Forms.Panel previewPanel;
		private System.Windows.Forms.Label cursorLabel;
		private System.Windows.Forms.Label formatLabel;
		private System.Windows.Forms.Label lodSizeLabel;
		private System.Windows.Forms.Label texSizeLabel;
		private System.Windows.Forms.Button clearButton;
		private System.Windows.Forms.Button browseButton;
		private System.Windows.Forms.ListBox lodList;
		private System.Windows.Forms.Panel imagePanel;
	}
}