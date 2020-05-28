namespace Hyperion
{
	partial class TextureImporter
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
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
			this.panel4 = new System.Windows.Forms.Panel();
			this.removeLODButton = new System.Windows.Forms.Button();
			this.addLODButton = new System.Windows.Forms.Button();
			this.label1 = new System.Windows.Forms.Label();
			this.panel2 = new System.Windows.Forms.Panel();
			this.panel5 = new System.Windows.Forms.Panel();
			this.totalSizeLabel = new System.Windows.Forms.Label();
			this.selectedLODLabel = new System.Windows.Forms.Label();
			this.clearLODButton = new System.Windows.Forms.Button();
			this.autogenButton = new System.Windows.Forms.Button();
			this.browseButton = new System.Windows.Forms.Button();
			this.imagePathBox = new System.Windows.Forms.TextBox();
			this.label5 = new System.Windows.Forms.Label();
			this.sizeLabel = new System.Windows.Forms.Label();
			this.resolutionLabel = new System.Windows.Forms.Label();
			this.panel6 = new System.Windows.Forms.Panel();
			this.resetButton = new System.Windows.Forms.Button();
			this.cancelButton = new System.Windows.Forms.Button();
			this.importButton = new System.Windows.Forms.Button();
			this.panel3 = new System.Windows.Forms.Panel();
			this.formatBox = new System.Windows.Forms.ComboBox();
			this.label3 = new System.Windows.Forms.Label();
			this.label2 = new System.Windows.Forms.Label();
			this.pathBox = new System.Windows.Forms.TextBox();
			this.panel7 = new System.Windows.Forms.Panel();
			this.panel1.SuspendLayout();
			this.panel4.SuspendLayout();
			this.panel2.SuspendLayout();
			this.panel5.SuspendLayout();
			this.panel6.SuspendLayout();
			this.panel3.SuspendLayout();
			this.SuspendLayout();
			// 
			// panel1
			// 
			this.panel1.Controls.Add(this.lodList);
			this.panel1.Controls.Add(this.panel4);
			this.panel1.Controls.Add(this.label1);
			this.panel1.Dock = System.Windows.Forms.DockStyle.Left;
			this.panel1.Location = new System.Drawing.Point(0, 0);
			this.panel1.Name = "panel1";
			this.panel1.Padding = new System.Windows.Forms.Padding(10);
			this.panel1.Size = new System.Drawing.Size(151, 615);
			this.panel1.TabIndex = 0;
			// 
			// lodList
			// 
			this.lodList.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(30)))), ((int)(((byte)(30)))), ((int)(((byte)(30)))));
			this.lodList.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.lodList.Dock = System.Windows.Forms.DockStyle.Fill;
			this.lodList.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.lodList.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(215)))), ((int)(((byte)(215)))), ((int)(((byte)(215)))));
			this.lodList.FormattingEnabled = true;
			this.lodList.ItemHeight = 15;
			this.lodList.Location = new System.Drawing.Point(10, 30);
			this.lodList.Name = "lodList";
			this.lodList.Size = new System.Drawing.Size(131, 546);
			this.lodList.TabIndex = 1;
			this.lodList.SelectedIndexChanged += new System.EventHandler(this.lodList_SelectedIndexChanged);
			// 
			// panel4
			// 
			this.panel4.Controls.Add(this.removeLODButton);
			this.panel4.Controls.Add(this.addLODButton);
			this.panel4.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.panel4.Location = new System.Drawing.Point(10, 576);
			this.panel4.Name = "panel4";
			this.panel4.Padding = new System.Windows.Forms.Padding(0, 5, 0, 0);
			this.panel4.Size = new System.Drawing.Size(131, 29);
			this.panel4.TabIndex = 2;
			// 
			// removeLODButton
			// 
			this.removeLODButton.AutoSize = true;
			this.removeLODButton.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(30)))), ((int)(((byte)(30)))), ((int)(((byte)(30)))));
			this.removeLODButton.Dock = System.Windows.Forms.DockStyle.Left;
			this.removeLODButton.FlatAppearance.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
			this.removeLODButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.removeLODButton.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.removeLODButton.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(215)))), ((int)(((byte)(215)))), ((int)(((byte)(215)))));
			this.removeLODButton.Location = new System.Drawing.Point(0, 5);
			this.removeLODButton.Name = "removeLODButton";
			this.removeLODButton.Size = new System.Drawing.Size(28, 24);
			this.removeLODButton.TabIndex = 1;
			this.removeLODButton.Text = "-";
			this.removeLODButton.UseVisualStyleBackColor = false;
			this.removeLODButton.Click += new System.EventHandler(this.removeLODButton_Click);
			// 
			// addLODButton
			// 
			this.addLODButton.AutoSize = true;
			this.addLODButton.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(30)))), ((int)(((byte)(30)))), ((int)(((byte)(30)))));
			this.addLODButton.Dock = System.Windows.Forms.DockStyle.Right;
			this.addLODButton.FlatAppearance.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
			this.addLODButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.addLODButton.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.addLODButton.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(215)))), ((int)(((byte)(215)))), ((int)(((byte)(215)))));
			this.addLODButton.Location = new System.Drawing.Point(103, 5);
			this.addLODButton.Name = "addLODButton";
			this.addLODButton.Size = new System.Drawing.Size(28, 24);
			this.addLODButton.TabIndex = 0;
			this.addLODButton.Text = "+";
			this.addLODButton.UseVisualStyleBackColor = false;
			this.addLODButton.Click += new System.EventHandler(this.addLODButton_Click);
			// 
			// label1
			// 
			this.label1.Dock = System.Windows.Forms.DockStyle.Top;
			this.label1.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.label1.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(215)))), ((int)(((byte)(215)))), ((int)(((byte)(215)))));
			this.label1.Location = new System.Drawing.Point(10, 10);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(131, 20);
			this.label1.TabIndex = 1;
			this.label1.Text = "LOD(s)";
			this.label1.TextAlign = System.Drawing.ContentAlignment.TopCenter;
			// 
			// panel2
			// 
			this.panel2.Controls.Add(this.panel5);
			this.panel2.Controls.Add(this.panel6);
			this.panel2.Controls.Add(this.panel3);
			this.panel2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel2.Location = new System.Drawing.Point(151, 0);
			this.panel2.Name = "panel2";
			this.panel2.Padding = new System.Windows.Forms.Padding(0, 10, 10, 10);
			this.panel2.Size = new System.Drawing.Size(663, 615);
			this.panel2.TabIndex = 1;
			// 
			// panel5
			// 
			this.panel5.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(30)))), ((int)(((byte)(30)))), ((int)(((byte)(30)))));
			this.panel5.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.panel5.Controls.Add(this.panel7);
			this.panel5.Controls.Add(this.totalSizeLabel);
			this.panel5.Controls.Add(this.selectedLODLabel);
			this.panel5.Controls.Add(this.clearLODButton);
			this.panel5.Controls.Add(this.autogenButton);
			this.panel5.Controls.Add(this.browseButton);
			this.panel5.Controls.Add(this.imagePathBox);
			this.panel5.Controls.Add(this.label5);
			this.panel5.Controls.Add(this.sizeLabel);
			this.panel5.Controls.Add(this.resolutionLabel);
			this.panel5.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panel5.Location = new System.Drawing.Point(0, 134);
			this.panel5.Name = "panel5";
			this.panel5.Padding = new System.Windows.Forms.Padding(10);
			this.panel5.Size = new System.Drawing.Size(653, 404);
			this.panel5.TabIndex = 1;
			// 
			// totalSizeLabel
			// 
			this.totalSizeLabel.Anchor = System.Windows.Forms.AnchorStyles.Top;
			this.totalSizeLabel.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.totalSizeLabel.Font = new System.Drawing.Font("Arial Rounded MT Bold", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.totalSizeLabel.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(215)))), ((int)(((byte)(215)))), ((int)(((byte)(215)))));
			this.totalSizeLabel.Location = new System.Drawing.Point(328, 6);
			this.totalSizeLabel.Name = "totalSizeLabel";
			this.totalSizeLabel.Size = new System.Drawing.Size(368, 31);
			this.totalSizeLabel.TabIndex = 11;
			this.totalSizeLabel.Text = "Total Raw Size: 0kb";
			this.totalSizeLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// selectedLODLabel
			// 
			this.selectedLODLabel.Anchor = System.Windows.Forms.AnchorStyles.Top;
			this.selectedLODLabel.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.selectedLODLabel.Font = new System.Drawing.Font("Arial Rounded MT Bold", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.selectedLODLabel.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(215)))), ((int)(((byte)(215)))), ((int)(((byte)(215)))));
			this.selectedLODLabel.Location = new System.Drawing.Point(-46, 6);
			this.selectedLODLabel.Name = "selectedLODLabel";
			this.selectedLODLabel.Size = new System.Drawing.Size(368, 31);
			this.selectedLODLabel.TabIndex = 10;
			this.selectedLODLabel.Text = "Selected LOD: <none>";
			this.selectedLODLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// clearLODButton
			// 
			this.clearLODButton.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
			this.clearLODButton.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(50)))), ((int)(((byte)(50)))), ((int)(((byte)(50)))));
			this.clearLODButton.FlatAppearance.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(100)))), ((int)(((byte)(100)))), ((int)(((byte)(100)))));
			this.clearLODButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.clearLODButton.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.clearLODButton.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(215)))), ((int)(((byte)(215)))), ((int)(((byte)(215)))));
			this.clearLODButton.Location = new System.Drawing.Point(400, 362);
			this.clearLODButton.Name = "clearLODButton";
			this.clearLODButton.Size = new System.Drawing.Size(100, 27);
			this.clearLODButton.TabIndex = 8;
			this.clearLODButton.Text = "Clear";
			this.clearLODButton.UseVisualStyleBackColor = false;
			this.clearLODButton.Click += new System.EventHandler(this.clearLODButton_Click);
			// 
			// autogenButton
			// 
			this.autogenButton.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
			this.autogenButton.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(50)))), ((int)(((byte)(50)))), ((int)(((byte)(50)))));
			this.autogenButton.FlatAppearance.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(100)))), ((int)(((byte)(100)))), ((int)(((byte)(100)))));
			this.autogenButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.autogenButton.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.autogenButton.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(215)))), ((int)(((byte)(215)))), ((int)(((byte)(215)))));
			this.autogenButton.Location = new System.Drawing.Point(248, 362);
			this.autogenButton.Name = "autogenButton";
			this.autogenButton.Size = new System.Drawing.Size(146, 27);
			this.autogenButton.TabIndex = 7;
			this.autogenButton.Text = "Generate MIPs";
			this.autogenButton.UseVisualStyleBackColor = false;
			this.autogenButton.Click += new System.EventHandler(this.autogenButton_Click);
			// 
			// browseButton
			// 
			this.browseButton.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
			this.browseButton.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(50)))), ((int)(((byte)(50)))), ((int)(((byte)(50)))));
			this.browseButton.FlatAppearance.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(100)))), ((int)(((byte)(100)))), ((int)(((byte)(100)))));
			this.browseButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.browseButton.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.browseButton.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(215)))), ((int)(((byte)(215)))), ((int)(((byte)(215)))));
			this.browseButton.Location = new System.Drawing.Point(142, 362);
			this.browseButton.Name = "browseButton";
			this.browseButton.Size = new System.Drawing.Size(100, 27);
			this.browseButton.TabIndex = 6;
			this.browseButton.Text = "Browse...";
			this.browseButton.UseVisualStyleBackColor = false;
			this.browseButton.Click += new System.EventHandler(this.browseButton_Click);
			// 
			// imagePathBox
			// 
			this.imagePathBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.imagePathBox.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(50)))), ((int)(((byte)(50)))), ((int)(((byte)(50)))));
			this.imagePathBox.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.imagePathBox.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.imagePathBox.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(215)))), ((int)(((byte)(215)))), ((int)(((byte)(215)))));
			this.imagePathBox.Location = new System.Drawing.Point(97, 330);
			this.imagePathBox.Name = "imagePathBox";
			this.imagePathBox.Size = new System.Drawing.Size(448, 23);
			this.imagePathBox.TabIndex = 4;
			// 
			// label5
			// 
			this.label5.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.label5.Font = new System.Drawing.Font("Arial Rounded MT Bold", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.label5.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(215)))), ((int)(((byte)(215)))), ((int)(((byte)(215)))));
			this.label5.Location = new System.Drawing.Point(10, 304);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(628, 23);
			this.label5.TabIndex = 5;
			this.label5.Text = "Image Data:";
			this.label5.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// sizeLabel
			// 
			this.sizeLabel.Anchor = System.Windows.Forms.AnchorStyles.Top;
			this.sizeLabel.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.sizeLabel.Font = new System.Drawing.Font("Arial Rounded MT Bold", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.sizeLabel.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(215)))), ((int)(((byte)(215)))), ((int)(((byte)(215)))));
			this.sizeLabel.Location = new System.Drawing.Point(-46, 37);
			this.sizeLabel.Name = "sizeLabel";
			this.sizeLabel.Size = new System.Drawing.Size(368, 31);
			this.sizeLabel.TabIndex = 2;
			this.sizeLabel.Text = "LOD Size: 0kb";
			this.sizeLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// resolutionLabel
			// 
			this.resolutionLabel.Anchor = System.Windows.Forms.AnchorStyles.Top;
			this.resolutionLabel.BackColor = System.Drawing.Color.Transparent;
			this.resolutionLabel.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.resolutionLabel.Font = new System.Drawing.Font("Arial Rounded MT Bold", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.resolutionLabel.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(215)))), ((int)(((byte)(215)))), ((int)(((byte)(215)))));
			this.resolutionLabel.Location = new System.Drawing.Point(328, 37);
			this.resolutionLabel.Name = "resolutionLabel";
			this.resolutionLabel.Size = new System.Drawing.Size(368, 31);
			this.resolutionLabel.TabIndex = 3;
			this.resolutionLabel.Text = "0px X 0px";
			this.resolutionLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// panel6
			// 
			this.panel6.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(30)))), ((int)(((byte)(30)))), ((int)(((byte)(30)))));
			this.panel6.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.panel6.Controls.Add(this.resetButton);
			this.panel6.Controls.Add(this.cancelButton);
			this.panel6.Controls.Add(this.importButton);
			this.panel6.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.panel6.Location = new System.Drawing.Point(0, 538);
			this.panel6.Name = "panel6";
			this.panel6.Size = new System.Drawing.Size(653, 67);
			this.panel6.TabIndex = 2;
			// 
			// resetButton
			// 
			this.resetButton.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
			this.resetButton.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(60)))), ((int)(((byte)(60)))), ((int)(((byte)(60)))));
			this.resetButton.FlatAppearance.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(100)))), ((int)(((byte)(100)))), ((int)(((byte)(100)))));
			this.resetButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.resetButton.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.resetButton.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(215)))), ((int)(((byte)(215)))), ((int)(((byte)(215)))));
			this.resetButton.Location = new System.Drawing.Point(272, 20);
			this.resetButton.Name = "resetButton";
			this.resetButton.Size = new System.Drawing.Size(96, 31);
			this.resetButton.TabIndex = 2;
			this.resetButton.Text = "Reset";
			this.resetButton.UseVisualStyleBackColor = false;
			this.resetButton.Click += new System.EventHandler(this.resetButton_Click);
			// 
			// cancelButton
			// 
			this.cancelButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.cancelButton.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(60)))), ((int)(((byte)(60)))), ((int)(((byte)(60)))));
			this.cancelButton.FlatAppearance.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(100)))), ((int)(((byte)(100)))), ((int)(((byte)(100)))));
			this.cancelButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.cancelButton.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.cancelButton.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(215)))), ((int)(((byte)(215)))), ((int)(((byte)(215)))));
			this.cancelButton.Location = new System.Drawing.Point(16, 20);
			this.cancelButton.Name = "cancelButton";
			this.cancelButton.Size = new System.Drawing.Size(96, 31);
			this.cancelButton.TabIndex = 1;
			this.cancelButton.Text = "Cancel";
			this.cancelButton.UseVisualStyleBackColor = false;
			this.cancelButton.Click += new System.EventHandler(this.cancelButton_Click);
			// 
			// importButton
			// 
			this.importButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.importButton.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(60)))), ((int)(((byte)(60)))), ((int)(((byte)(60)))));
			this.importButton.FlatAppearance.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(100)))), ((int)(((byte)(100)))), ((int)(((byte)(100)))));
			this.importButton.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.importButton.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.importButton.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(215)))), ((int)(((byte)(215)))), ((int)(((byte)(215)))));
			this.importButton.Location = new System.Drawing.Point(542, 20);
			this.importButton.Name = "importButton";
			this.importButton.Size = new System.Drawing.Size(96, 31);
			this.importButton.TabIndex = 0;
			this.importButton.Text = "Import";
			this.importButton.UseVisualStyleBackColor = false;
			this.importButton.Click += new System.EventHandler(this.importButton_Click);
			// 
			// panel3
			// 
			this.panel3.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(30)))), ((int)(((byte)(30)))), ((int)(((byte)(30)))));
			this.panel3.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.panel3.Controls.Add(this.formatBox);
			this.panel3.Controls.Add(this.label3);
			this.panel3.Controls.Add(this.label2);
			this.panel3.Controls.Add(this.pathBox);
			this.panel3.Dock = System.Windows.Forms.DockStyle.Top;
			this.panel3.Location = new System.Drawing.Point(0, 10);
			this.panel3.Name = "panel3";
			this.panel3.Size = new System.Drawing.Size(653, 124);
			this.panel3.TabIndex = 0;
			// 
			// formatBox
			// 
			this.formatBox.Anchor = System.Windows.Forms.AnchorStyles.Top;
			this.formatBox.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(50)))), ((int)(((byte)(50)))), ((int)(((byte)(50)))));
			this.formatBox.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.formatBox.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.formatBox.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(215)))), ((int)(((byte)(215)))), ((int)(((byte)(215)))));
			this.formatBox.FormattingEnabled = true;
			this.formatBox.Items.AddRange(new object[] {
            "Grayscale (no alpha) Uncompressed",
            "Grayscale (w/ alpha) Uncompressed",
            "RGBA Uncompressed",
            "sRGBA Uncompressed",
            "RGB DXT-1 Compressed",
            "RGBA DXT-5 Compressed",
            "RGB BC-7 Compressed",
            "RGBA BC-7 Compressed"});
			this.formatBox.Location = new System.Drawing.Point(127, 67);
			this.formatBox.Name = "formatBox";
			this.formatBox.Size = new System.Drawing.Size(267, 23);
			this.formatBox.TabIndex = 3;
			this.formatBox.Text = "Select...";
			// 
			// label3
			// 
			this.label3.Anchor = System.Windows.Forms.AnchorStyles.Top;
			this.label3.AutoSize = true;
			this.label3.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.label3.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(215)))), ((int)(((byte)(215)))), ((int)(((byte)(215)))));
			this.label3.Location = new System.Drawing.Point(51, 67);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(58, 15);
			this.label3.TabIndex = 2;
			this.label3.Text = "Format:";
			// 
			// label2
			// 
			this.label2.Anchor = System.Windows.Forms.AnchorStyles.Top;
			this.label2.AutoSize = true;
			this.label2.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.label2.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(215)))), ((int)(((byte)(215)))), ((int)(((byte)(215)))));
			this.label2.Location = new System.Drawing.Point(15, 23);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(94, 15);
			this.label2.TabIndex = 1;
			this.label2.Text = "Texture Path:";
			// 
			// pathBox
			// 
			this.pathBox.Anchor = System.Windows.Forms.AnchorStyles.Top;
			this.pathBox.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(50)))), ((int)(((byte)(50)))), ((int)(((byte)(50)))));
			this.pathBox.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.pathBox.Font = new System.Drawing.Font("Arial Rounded MT Bold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.pathBox.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(215)))), ((int)(((byte)(215)))), ((int)(((byte)(215)))));
			this.pathBox.Location = new System.Drawing.Point(127, 21);
			this.pathBox.Name = "pathBox";
			this.pathBox.Size = new System.Drawing.Size(509, 23);
			this.pathBox.TabIndex = 0;
			// 
			// panel7
			// 
			this.panel7.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.panel7.Location = new System.Drawing.Point(16, 79);
			this.panel7.Name = "panel7";
			this.panel7.Size = new System.Drawing.Size(622, 212);
			this.panel7.TabIndex = 12;
			this.panel7.Paint += new System.Windows.Forms.PaintEventHandler(this.panel7_Paint);
			this.panel7.Resize += new System.EventHandler(this.panel7_Resize);
			// 
			// TextureImporter
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(50)))), ((int)(((byte)(50)))), ((int)(((byte)(50)))));
			this.ClientSize = new System.Drawing.Size(814, 615);
			this.Controls.Add(this.panel2);
			this.Controls.Add(this.panel1);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.SizableToolWindow;
			this.MinimumSize = new System.Drawing.Size(830, 475);
			this.Name = "TextureImporter";
			this.Text = "Import Texture...";
			this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.TextureImport_FormClosing);
			this.Load += new System.EventHandler(this.TextureImport_Load);
			this.panel1.ResumeLayout(false);
			this.panel4.ResumeLayout(false);
			this.panel4.PerformLayout();
			this.panel2.ResumeLayout(false);
			this.panel5.ResumeLayout(false);
			this.panel5.PerformLayout();
			this.panel6.ResumeLayout(false);
			this.panel3.ResumeLayout(false);
			this.panel3.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.Panel panel1;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Panel panel2;
		private System.Windows.Forms.Panel panel3;
		public System.Windows.Forms.ComboBox formatBox;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Panel panel4;
		public System.Windows.Forms.Button removeLODButton;
		public System.Windows.Forms.Button addLODButton;
		public System.Windows.Forms.ListBox lodList;
		public System.Windows.Forms.TextBox pathBox;
		private System.Windows.Forms.Panel panel5;
		public System.Windows.Forms.Label sizeLabel;
		public System.Windows.Forms.Button browseButton;
		public System.Windows.Forms.TextBox imagePathBox;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.Panel panel6;
		public System.Windows.Forms.Button resetButton;
		public System.Windows.Forms.Button cancelButton;
		public System.Windows.Forms.Button importButton;
		public System.Windows.Forms.Button clearLODButton;
		public System.Windows.Forms.Button autogenButton;
		public System.Windows.Forms.Label totalSizeLabel;
		public System.Windows.Forms.Label selectedLODLabel;
		public System.Windows.Forms.Label resolutionLabel;
		private System.Windows.Forms.Panel panel7;
	}
}