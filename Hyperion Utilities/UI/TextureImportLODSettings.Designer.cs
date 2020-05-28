namespace Hyperion.UI
{
	partial class TextureImportLODSettings
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

		#region Component Designer generated code

		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.indexLabel = new System.Windows.Forms.Label();
			this.sourceLabel = new System.Windows.Forms.Label();
			this.colorOptions = new System.Windows.Forms.ComboBox();
			this.label2 = new System.Windows.Forms.Label();
			this.label3 = new System.Windows.Forms.Label();
			this.alphaOptions = new System.Windows.Forms.ComboBox();
			this.SuspendLayout();
			// 
			// indexLabel
			// 
			this.indexLabel.Dock = System.Windows.Forms.DockStyle.Top;
			this.indexLabel.Font = new System.Drawing.Font("Arial Rounded MT Bold", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.indexLabel.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(210)))), ((int)(((byte)(210)))), ((int)(((byte)(210)))));
			this.indexLabel.Location = new System.Drawing.Point(5, 5);
			this.indexLabel.Name = "indexLabel";
			this.indexLabel.Size = new System.Drawing.Size(220, 18);
			this.indexLabel.TabIndex = 0;
			this.indexLabel.Text = "LOD [0]";
			this.indexLabel.TextAlign = System.Drawing.ContentAlignment.TopCenter;
			// 
			// sourceLabel
			// 
			this.sourceLabel.Dock = System.Windows.Forms.DockStyle.Top;
			this.sourceLabel.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.sourceLabel.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(210)))), ((int)(((byte)(210)))), ((int)(((byte)(210)))));
			this.sourceLabel.Location = new System.Drawing.Point(5, 23);
			this.sourceLabel.Margin = new System.Windows.Forms.Padding(3);
			this.sourceLabel.Name = "sourceLabel";
			this.sourceLabel.Padding = new System.Windows.Forms.Padding(5, 5, 5, 0);
			this.sourceLabel.Size = new System.Drawing.Size(220, 27);
			this.sourceLabel.TabIndex = 1;
			this.sourceLabel.Text = "Source: 0x0 [RGBA]";
			this.sourceLabel.TextAlign = System.Drawing.ContentAlignment.TopCenter;
			// 
			// colorOptions
			// 
			this.colorOptions.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(60)))), ((int)(((byte)(60)))), ((int)(((byte)(60)))));
			this.colorOptions.Dock = System.Windows.Forms.DockStyle.Top;
			this.colorOptions.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.colorOptions.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.colorOptions.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(210)))), ((int)(((byte)(210)))), ((int)(((byte)(210)))));
			this.colorOptions.FormattingEnabled = true;
			this.colorOptions.Location = new System.Drawing.Point(5, 74);
			this.colorOptions.Name = "colorOptions";
			this.colorOptions.Size = new System.Drawing.Size(220, 23);
			this.colorOptions.TabIndex = 2;
			this.colorOptions.SelectedIndexChanged += new System.EventHandler(this.colorOptions_SelectedIndexChanged);
			// 
			// label2
			// 
			this.label2.Dock = System.Windows.Forms.DockStyle.Top;
			this.label2.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.label2.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(210)))), ((int)(((byte)(210)))), ((int)(((byte)(210)))));
			this.label2.Location = new System.Drawing.Point(5, 50);
			this.label2.Margin = new System.Windows.Forms.Padding(3);
			this.label2.Name = "label2";
			this.label2.Padding = new System.Windows.Forms.Padding(5, 5, 5, 0);
			this.label2.Size = new System.Drawing.Size(220, 24);
			this.label2.TabIndex = 3;
			this.label2.Text = "Color Options:";
			this.label2.TextAlign = System.Drawing.ContentAlignment.TopCenter;
			// 
			// label3
			// 
			this.label3.Dock = System.Windows.Forms.DockStyle.Top;
			this.label3.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.label3.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(210)))), ((int)(((byte)(210)))), ((int)(((byte)(210)))));
			this.label3.Location = new System.Drawing.Point(5, 97);
			this.label3.Margin = new System.Windows.Forms.Padding(3);
			this.label3.Name = "label3";
			this.label3.Padding = new System.Windows.Forms.Padding(5, 5, 5, 0);
			this.label3.Size = new System.Drawing.Size(220, 24);
			this.label3.TabIndex = 4;
			this.label3.Text = "Alpha Options:";
			this.label3.TextAlign = System.Drawing.ContentAlignment.TopCenter;
			// 
			// alphaOptions
			// 
			this.alphaOptions.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(60)))), ((int)(((byte)(60)))), ((int)(((byte)(60)))));
			this.alphaOptions.Dock = System.Windows.Forms.DockStyle.Top;
			this.alphaOptions.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.alphaOptions.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.alphaOptions.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(210)))), ((int)(((byte)(210)))), ((int)(((byte)(210)))));
			this.alphaOptions.FormattingEnabled = true;
			this.alphaOptions.Location = new System.Drawing.Point(5, 121);
			this.alphaOptions.Margin = new System.Windows.Forms.Padding(10);
			this.alphaOptions.Name = "alphaOptions";
			this.alphaOptions.Size = new System.Drawing.Size(220, 23);
			this.alphaOptions.TabIndex = 5;
			this.alphaOptions.SelectedIndexChanged += new System.EventHandler(this.alphaOptions_SelectedIndexChanged);
			// 
			// TextureImportLODSettings
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(30)))), ((int)(((byte)(30)))), ((int)(((byte)(30)))));
			this.Controls.Add(this.alphaOptions);
			this.Controls.Add(this.label3);
			this.Controls.Add(this.colorOptions);
			this.Controls.Add(this.label2);
			this.Controls.Add(this.sourceLabel);
			this.Controls.Add(this.indexLabel);
			this.Margin = new System.Windows.Forms.Padding(5);
			this.MaximumSize = new System.Drawing.Size(230, 156);
			this.MinimumSize = new System.Drawing.Size(230, 156);
			this.Name = "TextureImportLODSettings";
			this.Padding = new System.Windows.Forms.Padding(5);
			this.Size = new System.Drawing.Size(230, 156);
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.Label indexLabel;
		private System.Windows.Forms.Label sourceLabel;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label label3;
		public System.Windows.Forms.ComboBox colorOptions;
		public System.Windows.Forms.ComboBox alphaOptions;
	}
}
