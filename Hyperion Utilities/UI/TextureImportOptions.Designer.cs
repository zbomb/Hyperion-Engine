namespace Hyperion
{
	partial class TextureImportOptions
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
			this.filenameLabel = new System.Windows.Forms.Label();
			this.lodCountLabel = new System.Windows.Forms.Label();
			this.fullResLabel = new System.Windows.Forms.Label();
			this.sizeLabel = new System.Windows.Forms.Label();
			this.formatLabel = new System.Windows.Forms.Label();
			this.panel2 = new System.Windows.Forms.Panel();
			this.importButton = new System.Windows.Forms.Button();
			this.cancelButton = new System.Windows.Forms.Button();
			this.lodList = new System.Windows.Forms.FlowLayoutPanel();
			this.panel1.SuspendLayout();
			this.panel2.SuspendLayout();
			this.SuspendLayout();
			// 
			// panel1
			// 
			this.panel1.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(30)))), ((int)(((byte)(30)))), ((int)(((byte)(30)))));
			this.panel1.Controls.Add(this.formatLabel);
			this.panel1.Controls.Add(this.sizeLabel);
			this.panel1.Controls.Add(this.fullResLabel);
			this.panel1.Controls.Add(this.lodCountLabel);
			this.panel1.Controls.Add(this.filenameLabel);
			this.panel1.Dock = System.Windows.Forms.DockStyle.Top;
			this.panel1.Location = new System.Drawing.Point(0, 0);
			this.panel1.Name = "panel1";
			this.panel1.Padding = new System.Windows.Forms.Padding(0, 10, 0, 0);
			this.panel1.Size = new System.Drawing.Size(816, 121);
			this.panel1.TabIndex = 0;
			// 
			// filenameLabel
			// 
			this.filenameLabel.Dock = System.Windows.Forms.DockStyle.Top;
			this.filenameLabel.Font = new System.Drawing.Font("Arial Rounded MT Bold", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.filenameLabel.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(210)))), ((int)(((byte)(210)))), ((int)(((byte)(210)))));
			this.filenameLabel.Location = new System.Drawing.Point(0, 10);
			this.filenameLabel.Name = "filenameLabel";
			this.filenameLabel.Size = new System.Drawing.Size(816, 30);
			this.filenameLabel.TabIndex = 0;
			this.filenameLabel.Text = "Output File: ";
			this.filenameLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// lodCountLabel
			// 
			this.lodCountLabel.Font = new System.Drawing.Font("Arial Rounded MT Bold", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.lodCountLabel.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(210)))), ((int)(((byte)(210)))), ((int)(((byte)(210)))));
			this.lodCountLabel.Location = new System.Drawing.Point(0, 47);
			this.lodCountLabel.Name = "lodCountLabel";
			this.lodCountLabel.Size = new System.Drawing.Size(408, 30);
			this.lodCountLabel.TabIndex = 1;
			this.lodCountLabel.Text = "LOD Count: ";
			this.lodCountLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// fullResLabel
			// 
			this.fullResLabel.Font = new System.Drawing.Font("Arial Rounded MT Bold", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.fullResLabel.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(210)))), ((int)(((byte)(210)))), ((int)(((byte)(210)))));
			this.fullResLabel.Location = new System.Drawing.Point(408, 47);
			this.fullResLabel.Name = "fullResLabel";
			this.fullResLabel.Size = new System.Drawing.Size(408, 30);
			this.fullResLabel.TabIndex = 2;
			this.fullResLabel.Text = "Full Res:";
			this.fullResLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// sizeLabel
			// 
			this.sizeLabel.Font = new System.Drawing.Font("Arial Rounded MT Bold", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.sizeLabel.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(210)))), ((int)(((byte)(210)))), ((int)(((byte)(210)))));
			this.sizeLabel.Location = new System.Drawing.Point(0, 77);
			this.sizeLabel.Name = "sizeLabel";
			this.sizeLabel.Size = new System.Drawing.Size(408, 30);
			this.sizeLabel.TabIndex = 3;
			this.sizeLabel.Text = "Size:";
			this.sizeLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// formatLabel
			// 
			this.formatLabel.Font = new System.Drawing.Font("Arial Rounded MT Bold", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.formatLabel.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(210)))), ((int)(((byte)(210)))), ((int)(((byte)(210)))));
			this.formatLabel.Location = new System.Drawing.Point(408, 77);
			this.formatLabel.Name = "formatLabel";
			this.formatLabel.Size = new System.Drawing.Size(408, 30);
			this.formatLabel.TabIndex = 4;
			this.formatLabel.Text = "Format: ";
			this.formatLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// panel2
			// 
			this.panel2.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(30)))), ((int)(((byte)(30)))), ((int)(((byte)(30)))));
			this.panel2.Controls.Add(this.cancelButton);
			this.panel2.Controls.Add(this.importButton);
			this.panel2.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.panel2.Location = new System.Drawing.Point(0, 451);
			this.panel2.Name = "panel2";
			this.panel2.Size = new System.Drawing.Size(816, 48);
			this.panel2.TabIndex = 1;
			// 
			// importButton
			// 
			this.importButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.importButton.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(60)))), ((int)(((byte)(60)))), ((int)(((byte)(60)))));
			this.importButton.FlatAppearance.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(100)))), ((int)(((byte)(100)))), ((int)(((byte)(100)))));
			this.importButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.importButton.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.importButton.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(215)))), ((int)(((byte)(215)))), ((int)(((byte)(215)))));
			this.importButton.Location = new System.Drawing.Point(708, 9);
			this.importButton.Name = "importButton";
			this.importButton.Size = new System.Drawing.Size(96, 31);
			this.importButton.TabIndex = 1;
			this.importButton.Text = "Import";
			this.importButton.UseVisualStyleBackColor = false;
			this.importButton.Click += new System.EventHandler(this.importButton_Click);
			// 
			// cancelButton
			// 
			this.cancelButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.cancelButton.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(60)))), ((int)(((byte)(60)))), ((int)(((byte)(60)))));
			this.cancelButton.FlatAppearance.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(100)))), ((int)(((byte)(100)))), ((int)(((byte)(100)))));
			this.cancelButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.cancelButton.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.cancelButton.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(215)))), ((int)(((byte)(215)))), ((int)(((byte)(215)))));
			this.cancelButton.Location = new System.Drawing.Point(12, 9);
			this.cancelButton.Name = "cancelButton";
			this.cancelButton.Size = new System.Drawing.Size(96, 31);
			this.cancelButton.TabIndex = 2;
			this.cancelButton.Text = "Cancel";
			this.cancelButton.UseVisualStyleBackColor = false;
			this.cancelButton.Click += new System.EventHandler(this.cancelButton_Click);
			// 
			// lodList
			// 
			this.lodList.Dock = System.Windows.Forms.DockStyle.Fill;
			this.lodList.Location = new System.Drawing.Point(0, 121);
			this.lodList.Name = "lodList";
			this.lodList.Size = new System.Drawing.Size(816, 330);
			this.lodList.TabIndex = 2;
			this.lodList.Paint += new System.Windows.Forms.PaintEventHandler(this.lodList_Paint);
			// 
			// TextureImportOptions
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(50)))), ((int)(((byte)(50)))), ((int)(((byte)(50)))));
			this.ClientSize = new System.Drawing.Size(816, 499);
			this.Controls.Add(this.lodList);
			this.Controls.Add(this.panel2);
			this.Controls.Add(this.panel1);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.SizableToolWindow;
			this.Name = "TextureImportOptions";
			this.Text = "TextureImportOptions";
			this.panel1.ResumeLayout(false);
			this.panel2.ResumeLayout(false);
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.Panel panel1;
		private System.Windows.Forms.Label formatLabel;
		private System.Windows.Forms.Label sizeLabel;
		private System.Windows.Forms.Label fullResLabel;
		private System.Windows.Forms.Label lodCountLabel;
		private System.Windows.Forms.Label filenameLabel;
		private System.Windows.Forms.Panel panel2;
		public System.Windows.Forms.Button cancelButton;
		public System.Windows.Forms.Button importButton;
		private System.Windows.Forms.FlowLayoutPanel lodList;
	}
}