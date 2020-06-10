namespace Hyperion
{
	partial class ZoomControl
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
			this.zoomInButton = new System.Windows.Forms.Button();
			this.zoomOutButton = new System.Windows.Forms.Button();
			this.percentLabel = new System.Windows.Forms.Label();
			this.SuspendLayout();
			// 
			// zoomInButton
			// 
			this.zoomInButton.BackgroundImage = global::Hyperion.Properties.Resources.zoom_in;
			this.zoomInButton.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Zoom;
			this.zoomInButton.Dock = System.Windows.Forms.DockStyle.Left;
			this.zoomInButton.FlatAppearance.BorderSize = 0;
			this.zoomInButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.zoomInButton.ForeColor = System.Drawing.Color.Black;
			this.zoomInButton.Location = new System.Drawing.Point(0, 0);
			this.zoomInButton.Name = "zoomInButton";
			this.zoomInButton.Padding = new System.Windows.Forms.Padding(1);
			this.zoomInButton.Size = new System.Drawing.Size(24, 22);
			this.zoomInButton.TabIndex = 0;
			this.zoomInButton.UseVisualStyleBackColor = true;
			this.zoomInButton.Click += new System.EventHandler(this.zoomInButton_Click);
			// 
			// zoomOutButton
			// 
			this.zoomOutButton.BackgroundImage = global::Hyperion.Properties.Resources.zoom_out;
			this.zoomOutButton.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Zoom;
			this.zoomOutButton.Dock = System.Windows.Forms.DockStyle.Right;
			this.zoomOutButton.FlatAppearance.BorderSize = 0;
			this.zoomOutButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.zoomOutButton.ForeColor = System.Drawing.Color.Black;
			this.zoomOutButton.Location = new System.Drawing.Point(84, 0);
			this.zoomOutButton.Name = "zoomOutButton";
			this.zoomOutButton.Padding = new System.Windows.Forms.Padding(1);
			this.zoomOutButton.Size = new System.Drawing.Size(24, 22);
			this.zoomOutButton.TabIndex = 1;
			this.zoomOutButton.UseVisualStyleBackColor = true;
			this.zoomOutButton.Click += new System.EventHandler(this.zoomOutButton_Click);
			// 
			// percentLabel
			// 
			this.percentLabel.Dock = System.Windows.Forms.DockStyle.Fill;
			this.percentLabel.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.percentLabel.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(215)))), ((int)(((byte)(215)))), ((int)(((byte)(215)))));
			this.percentLabel.Location = new System.Drawing.Point(24, 0);
			this.percentLabel.Name = "percentLabel";
			this.percentLabel.Size = new System.Drawing.Size(60, 22);
			this.percentLabel.TabIndex = 2;
			this.percentLabel.Text = "100%";
			this.percentLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			this.percentLabel.DoubleClick += new System.EventHandler(this.percentLabel_DoubleClick);
			// 
			// ZoomControl
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.BackColor = System.Drawing.Color.Transparent;
			this.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.Controls.Add(this.percentLabel);
			this.Controls.Add(this.zoomOutButton);
			this.Controls.Add(this.zoomInButton);
			this.MinimumSize = new System.Drawing.Size(110, 24);
			this.Name = "ZoomControl";
			this.Size = new System.Drawing.Size(108, 22);
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.Button zoomInButton;
		private System.Windows.Forms.Button zoomOutButton;
		private System.Windows.Forms.Label percentLabel;
	}
}
