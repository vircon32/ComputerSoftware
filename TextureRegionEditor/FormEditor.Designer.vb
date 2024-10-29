<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()> _
Partial Class FormEditor
    Inherits System.Windows.Forms.Form

    'Form reemplaza a Dispose para limpiar la lista de componentes.
    <System.Diagnostics.DebuggerNonUserCode()> _
    Protected Overrides Sub Dispose(ByVal disposing As Boolean)
        Try
            If disposing AndAlso components IsNot Nothing Then
                components.Dispose()
            End If
        Finally
            MyBase.Dispose(disposing)
        End Try
    End Sub

    'Requerido por el Diseñador de Windows Forms
    Private components As System.ComponentModel.IContainer

    'NOTA: el Diseñador de Windows Forms necesita el siguiente procedimiento
    'Se puede modificar usando el Diseñador de Windows Forms.  
    'No lo modifique con el editor de código.
    <System.Diagnostics.DebuggerStepThrough()> _
    Private Sub InitializeComponent()
        Me.components = New System.ComponentModel.Container()
        Dim resources As System.ComponentModel.ComponentResourceManager = New System.ComponentModel.ComponentResourceManager(GetType(FormEditor))
        Me.ButtonDeleteRegion = New System.Windows.Forms.Button()
        Me.ButtonNewRegion = New System.Windows.Forms.Button()
        Me.Label1 = New System.Windows.Forms.Label()
        Me.TB_Left = New System.Windows.Forms.TextBox()
        Me.ButtonLoadTexture = New System.Windows.Forms.Button()
        Me.ButtonZoomMinus = New System.Windows.Forms.Button()
        Me.ButtonZoomPlus = New System.Windows.Forms.Button()
        Me.OpenDialog = New System.Windows.Forms.OpenFileDialog()
        Me.PanelTexture = New System.Windows.Forms.Panel()
        Me.PictureTexture = New RegionEditor.CustomPictureBox()
        Me.PanelRegionLists = New System.Windows.Forms.Panel()
        Me.TabRegionTypes = New System.Windows.Forms.TabControl()
        Me.TabRegions = New System.Windows.Forms.TabPage()
        Me.ButtonRegionUp = New System.Windows.Forms.Button()
        Me.ButtonRegionDown = New System.Windows.Forms.Button()
        Me.LB_Regions = New System.Windows.Forms.ListBox()
        Me.ButtonEditRegion = New System.Windows.Forms.Button()
        Me.Label17 = New System.Windows.Forms.Label()
        Me.TabMatrices = New System.Windows.Forms.TabPage()
        Me.ButtonMatrixUp = New System.Windows.Forms.Button()
        Me.ButtonMatrixDown = New System.Windows.Forms.Button()
        Me.LB_Matrices = New System.Windows.Forms.ListBox()
        Me.ButtonDeleteMatrix = New System.Windows.Forms.Button()
        Me.ButtonEditMatrix = New System.Windows.Forms.Button()
        Me.ButtonNewMatrix = New System.Windows.Forms.Button()
        Me.Label18 = New System.Windows.Forms.Label()
        Me.Label3 = New System.Windows.Forms.Label()
        Me.TB_Right = New System.Windows.Forms.TextBox()
        Me.Label4 = New System.Windows.Forms.Label()
        Me.TB_Top = New System.Windows.Forms.TextBox()
        Me.Label5 = New System.Windows.Forms.Label()
        Me.Label6 = New System.Windows.Forms.Label()
        Me.Label7 = New System.Windows.Forms.Label()
        Me.TB_Bottom = New System.Windows.Forms.TextBox()
        Me.Label10 = New System.Windows.Forms.Label()
        Me.PanelLimits = New System.Windows.Forms.Panel()
        Me.Panel4 = New System.Windows.Forms.Panel()
        Me.ButtonColorHotspot = New System.Windows.Forms.Button()
        Me.ButtonColorRegion = New System.Windows.Forms.Button()
        Me.LabelZoom = New System.Windows.Forms.Label()
        Me.TB_TextureName = New System.Windows.Forms.TextBox()
        Me.Label25 = New System.Windows.Forms.Label()
        Me.Label11 = New System.Windows.Forms.Label()
        Me.BlinkTimer = New System.Windows.Forms.Timer(Me.components)
        Me.PanelHotspot = New System.Windows.Forms.Panel()
        Me.TB_HotspotX = New System.Windows.Forms.TextBox()
        Me.TB_HotspotY = New System.Windows.Forms.TextBox()
        Me.Label8 = New System.Windows.Forms.Label()
        Me.Label12 = New System.Windows.Forms.Label()
        Me.Label13 = New System.Windows.Forms.Label()
        Me.PanelMatrix = New System.Windows.Forms.Panel()
        Me.TB_Rows = New System.Windows.Forms.TextBox()
        Me.TB_Separation = New System.Windows.Forms.TextBox()
        Me.TB_Columns = New System.Windows.Forms.TextBox()
        Me.Label15 = New System.Windows.Forms.Label()
        Me.Label2 = New System.Windows.Forms.Label()
        Me.Label9 = New System.Windows.Forms.Label()
        Me.Label14 = New System.Windows.Forms.Label()
        Me.PanelProject = New System.Windows.Forms.Panel()
        Me.ButtonExportHeader = New System.Windows.Forms.Button()
        Me.ButtonSaveProject = New System.Windows.Forms.Button()
        Me.ButtonLoadProject = New System.Windows.Forms.Button()
        Me.Label16 = New System.Windows.Forms.Label()
        Me.PanelTools = New System.Windows.Forms.Panel()
        Me.Label24 = New System.Windows.Forms.Label()
        Me.Label23 = New System.Windows.Forms.Label()
        Me.Label22 = New System.Windows.Forms.Label()
        Me.Label21 = New System.Windows.Forms.Label()
        Me.Label19 = New System.Windows.Forms.Label()
        Me.RB_Pan = New System.Windows.Forms.RadioButton()
        Me.RB_MoveRegion = New System.Windows.Forms.RadioButton()
        Me.RB_Hotspot = New System.Windows.Forms.RadioButton()
        Me.RB_BottomRight = New System.Windows.Forms.RadioButton()
        Me.RB_TopLeft = New System.Windows.Forms.RadioButton()
        Me.Label20 = New System.Windows.Forms.Label()
        Me.TTHelp = New System.Windows.Forms.ToolTip(Me.components)
        Me.SaveDialog = New System.Windows.Forms.SaveFileDialog()
        Me.ColorPicker = New System.Windows.Forms.ColorDialog()
        Me.ButtonExportASM = New System.Windows.Forms.Button()
        Me.PanelTexture.SuspendLayout()
        CType(Me.PictureTexture, System.ComponentModel.ISupportInitialize).BeginInit()
        Me.PanelRegionLists.SuspendLayout()
        Me.TabRegionTypes.SuspendLayout()
        Me.TabRegions.SuspendLayout()
        Me.TabMatrices.SuspendLayout()
        Me.PanelLimits.SuspendLayout()
        Me.Panel4.SuspendLayout()
        Me.PanelHotspot.SuspendLayout()
        Me.PanelMatrix.SuspendLayout()
        Me.PanelProject.SuspendLayout()
        Me.PanelTools.SuspendLayout()
        Me.SuspendLayout()
        '
        'ButtonDeleteRegion
        '
        Me.ButtonDeleteRegion.Anchor = CType(((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Left) _
            Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.ButtonDeleteRegion.Enabled = False
        Me.ButtonDeleteRegion.Image = CType(resources.GetObject("ButtonDeleteRegion.Image"), System.Drawing.Image)
        Me.ButtonDeleteRegion.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft
        Me.ButtonDeleteRegion.Location = New System.Drawing.Point(204, 50)
        Me.ButtonDeleteRegion.Margin = New System.Windows.Forms.Padding(4)
        Me.ButtonDeleteRegion.Name = "ButtonDeleteRegion"
        Me.ButtonDeleteRegion.Size = New System.Drawing.Size(90, 40)
        Me.ButtonDeleteRegion.TabIndex = 8
        Me.ButtonDeleteRegion.Text = "Delete"
        Me.ButtonDeleteRegion.TextAlign = System.Drawing.ContentAlignment.MiddleRight
        Me.ButtonDeleteRegion.UseVisualStyleBackColor = True
        '
        'ButtonNewRegion
        '
        Me.ButtonNewRegion.Image = CType(resources.GetObject("ButtonNewRegion.Image"), System.Drawing.Image)
        Me.ButtonNewRegion.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft
        Me.ButtonNewRegion.Location = New System.Drawing.Point(2, 50)
        Me.ButtonNewRegion.Margin = New System.Windows.Forms.Padding(4)
        Me.ButtonNewRegion.Name = "ButtonNewRegion"
        Me.ButtonNewRegion.Size = New System.Drawing.Size(80, 40)
        Me.ButtonNewRegion.TabIndex = 7
        Me.ButtonNewRegion.Text = "New"
        Me.ButtonNewRegion.TextAlign = System.Drawing.ContentAlignment.MiddleRight
        Me.ButtonNewRegion.UseVisualStyleBackColor = True
        '
        'Label1
        '
        Me.Label1.Anchor = CType(((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Left) _
            Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.Label1.BackColor = System.Drawing.Color.Silver
        Me.Label1.Font = New System.Drawing.Font("Microsoft Sans Serif", 14.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label1.Location = New System.Drawing.Point(4, 6)
        Me.Label1.Margin = New System.Windows.Forms.Padding(4, 0, 4, 0)
        Me.Label1.Name = "Label1"
        Me.Label1.Size = New System.Drawing.Size(293, 31)
        Me.Label1.TabIndex = 9
        Me.Label1.Text = "Defined regions"
        Me.Label1.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        '
        'TB_Left
        '
        Me.TB_Left.Font = New System.Drawing.Font("Microsoft Sans Serif", 12.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.TB_Left.Location = New System.Drawing.Point(100, 53)
        Me.TB_Left.Name = "TB_Left"
        Me.TB_Left.Size = New System.Drawing.Size(80, 26)
        Me.TB_Left.TabIndex = 8
        Me.TB_Left.Text = "0"
        '
        'ButtonLoadTexture
        '
        Me.ButtonLoadTexture.Image = CType(resources.GetObject("ButtonLoadTexture.Image"), System.Drawing.Image)
        Me.ButtonLoadTexture.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft
        Me.ButtonLoadTexture.Location = New System.Drawing.Point(8, 43)
        Me.ButtonLoadTexture.Name = "ButtonLoadTexture"
        Me.ButtonLoadTexture.Size = New System.Drawing.Size(159, 43)
        Me.ButtonLoadTexture.TabIndex = 9
        Me.ButtonLoadTexture.Text = "Load PNG texture"
        Me.ButtonLoadTexture.TextAlign = System.Drawing.ContentAlignment.MiddleRight
        Me.ButtonLoadTexture.UseVisualStyleBackColor = True
        '
        'ButtonZoomMinus
        '
        Me.ButtonZoomMinus.Image = CType(resources.GetObject("ButtonZoomMinus.Image"), System.Drawing.Image)
        Me.ButtonZoomMinus.Location = New System.Drawing.Point(61, 197)
        Me.ButtonZoomMinus.Name = "ButtonZoomMinus"
        Me.ButtonZoomMinus.Size = New System.Drawing.Size(50, 45)
        Me.ButtonZoomMinus.TabIndex = 9
        Me.ButtonZoomMinus.TextAlign = System.Drawing.ContentAlignment.TopCenter
        Me.ButtonZoomMinus.UseVisualStyleBackColor = True
        '
        'ButtonZoomPlus
        '
        Me.ButtonZoomPlus.Image = CType(resources.GetObject("ButtonZoomPlus.Image"), System.Drawing.Image)
        Me.ButtonZoomPlus.Location = New System.Drawing.Point(117, 197)
        Me.ButtonZoomPlus.Name = "ButtonZoomPlus"
        Me.ButtonZoomPlus.Size = New System.Drawing.Size(50, 45)
        Me.ButtonZoomPlus.TabIndex = 9
        Me.ButtonZoomPlus.TextAlign = System.Drawing.ContentAlignment.TopCenter
        Me.ButtonZoomPlus.UseVisualStyleBackColor = True
        '
        'PanelTexture
        '
        Me.PanelTexture.Anchor = CType((((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Bottom) _
            Or System.Windows.Forms.AnchorStyles.Left) _
            Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.PanelTexture.AutoScroll = True
        Me.PanelTexture.AutoScrollMargin = New System.Drawing.Size(10, 10)
        Me.PanelTexture.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink
        Me.PanelTexture.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle
        Me.PanelTexture.Controls.Add(Me.PictureTexture)
        Me.PanelTexture.Enabled = False
        Me.PanelTexture.Location = New System.Drawing.Point(823, 12)
        Me.PanelTexture.Name = "PanelTexture"
        Me.PanelTexture.Size = New System.Drawing.Size(487, 484)
        Me.PanelTexture.TabIndex = 10
        '
        'PictureTexture
        '
        Me.PictureTexture.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch
        Me.PictureTexture.Location = New System.Drawing.Point(0, 0)
        Me.PictureTexture.Margin = New System.Windows.Forms.Padding(4)
        Me.PictureTexture.Name = "PictureTexture"
        Me.PictureTexture.Size = New System.Drawing.Size(100, 100)
        Me.PictureTexture.TabIndex = 0
        Me.PictureTexture.TabStop = False
        Me.PictureTexture.ZoomLevel = 1
        '
        'PanelRegionLists
        '
        Me.PanelRegionLists.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle
        Me.PanelRegionLists.Controls.Add(Me.TabRegionTypes)
        Me.PanelRegionLists.Controls.Add(Me.Label1)
        Me.PanelRegionLists.Enabled = False
        Me.PanelRegionLists.Location = New System.Drawing.Point(12, 12)
        Me.PanelRegionLists.Name = "PanelRegionLists"
        Me.PanelRegionLists.Size = New System.Drawing.Size(303, 484)
        Me.PanelRegionLists.TabIndex = 11
        '
        'TabRegionTypes
        '
        Me.TabRegionTypes.Anchor = CType((((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Bottom) _
            Or System.Windows.Forms.AnchorStyles.Left) _
            Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.TabRegionTypes.Controls.Add(Me.TabRegions)
        Me.TabRegionTypes.Controls.Add(Me.TabMatrices)
        Me.TabRegionTypes.Location = New System.Drawing.Point(0, 44)
        Me.TabRegionTypes.Name = "TabRegionTypes"
        Me.TabRegionTypes.SelectedIndex = 0
        Me.TabRegionTypes.Size = New System.Drawing.Size(302, 439)
        Me.TabRegionTypes.TabIndex = 10
        '
        'TabRegions
        '
        Me.TabRegions.Controls.Add(Me.ButtonRegionUp)
        Me.TabRegions.Controls.Add(Me.ButtonRegionDown)
        Me.TabRegions.Controls.Add(Me.LB_Regions)
        Me.TabRegions.Controls.Add(Me.ButtonDeleteRegion)
        Me.TabRegions.Controls.Add(Me.ButtonEditRegion)
        Me.TabRegions.Controls.Add(Me.ButtonNewRegion)
        Me.TabRegions.Controls.Add(Me.Label17)
        Me.TabRegions.Location = New System.Drawing.Point(4, 25)
        Me.TabRegions.Name = "TabRegions"
        Me.TabRegions.Padding = New System.Windows.Forms.Padding(3)
        Me.TabRegions.Size = New System.Drawing.Size(294, 410)
        Me.TabRegions.TabIndex = 0
        Me.TabRegions.Text = "Single regions"
        Me.TabRegions.UseVisualStyleBackColor = True
        '
        'ButtonRegionUp
        '
        Me.ButtonRegionUp.Enabled = False
        Me.ButtonRegionUp.Location = New System.Drawing.Point(163, 50)
        Me.ButtonRegionUp.Name = "ButtonRegionUp"
        Me.ButtonRegionUp.Size = New System.Drawing.Size(40, 20)
        Me.ButtonRegionUp.TabIndex = 20
        Me.ButtonRegionUp.Text = "▲"
        Me.ButtonRegionUp.UseVisualStyleBackColor = True
        '
        'ButtonRegionDown
        '
        Me.ButtonRegionDown.Enabled = False
        Me.ButtonRegionDown.Location = New System.Drawing.Point(163, 70)
        Me.ButtonRegionDown.Name = "ButtonRegionDown"
        Me.ButtonRegionDown.Size = New System.Drawing.Size(40, 20)
        Me.ButtonRegionDown.TabIndex = 20
        Me.ButtonRegionDown.Text = "▼"
        Me.ButtonRegionDown.UseVisualStyleBackColor = True
        '
        'LB_Regions
        '
        Me.LB_Regions.Anchor = CType((((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Bottom) _
            Or System.Windows.Forms.AnchorStyles.Left) _
            Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.LB_Regions.FormattingEnabled = True
        Me.LB_Regions.ItemHeight = 16
        Me.LB_Regions.Location = New System.Drawing.Point(0, 102)
        Me.LB_Regions.Name = "LB_Regions"
        Me.LB_Regions.Size = New System.Drawing.Size(294, 308)
        Me.LB_Regions.TabIndex = 19
        '
        'ButtonEditRegion
        '
        Me.ButtonEditRegion.Enabled = False
        Me.ButtonEditRegion.Image = CType(resources.GetObject("ButtonEditRegion.Image"), System.Drawing.Image)
        Me.ButtonEditRegion.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft
        Me.ButtonEditRegion.Location = New System.Drawing.Point(82, 50)
        Me.ButtonEditRegion.Margin = New System.Windows.Forms.Padding(4)
        Me.ButtonEditRegion.Name = "ButtonEditRegion"
        Me.ButtonEditRegion.Size = New System.Drawing.Size(80, 40)
        Me.ButtonEditRegion.TabIndex = 7
        Me.ButtonEditRegion.Text = "Edit"
        Me.ButtonEditRegion.TextAlign = System.Drawing.ContentAlignment.MiddleRight
        Me.ButtonEditRegion.UseVisualStyleBackColor = True
        '
        'Label17
        '
        Me.Label17.Location = New System.Drawing.Point(6, 7)
        Me.Label17.Name = "Label17"
        Me.Label17.Size = New System.Drawing.Size(282, 38)
        Me.Label17.TabIndex = 12
        Me.Label17.Text = "This is the list of single regions defined for this texture. Use these buttons to" & _
    " edit the list"
        Me.Label17.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        '
        'TabMatrices
        '
        Me.TabMatrices.Controls.Add(Me.ButtonMatrixUp)
        Me.TabMatrices.Controls.Add(Me.ButtonMatrixDown)
        Me.TabMatrices.Controls.Add(Me.LB_Matrices)
        Me.TabMatrices.Controls.Add(Me.ButtonDeleteMatrix)
        Me.TabMatrices.Controls.Add(Me.ButtonEditMatrix)
        Me.TabMatrices.Controls.Add(Me.ButtonNewMatrix)
        Me.TabMatrices.Controls.Add(Me.Label18)
        Me.TabMatrices.Location = New System.Drawing.Point(4, 25)
        Me.TabMatrices.Name = "TabMatrices"
        Me.TabMatrices.Padding = New System.Windows.Forms.Padding(3)
        Me.TabMatrices.Size = New System.Drawing.Size(294, 410)
        Me.TabMatrices.TabIndex = 1
        Me.TabMatrices.Text = "Region matrices"
        Me.TabMatrices.UseVisualStyleBackColor = True
        '
        'ButtonMatrixUp
        '
        Me.ButtonMatrixUp.Enabled = False
        Me.ButtonMatrixUp.Location = New System.Drawing.Point(163, 50)
        Me.ButtonMatrixUp.Name = "ButtonMatrixUp"
        Me.ButtonMatrixUp.Size = New System.Drawing.Size(40, 20)
        Me.ButtonMatrixUp.TabIndex = 21
        Me.ButtonMatrixUp.Text = "▲"
        Me.ButtonMatrixUp.UseVisualStyleBackColor = True
        '
        'ButtonMatrixDown
        '
        Me.ButtonMatrixDown.Enabled = False
        Me.ButtonMatrixDown.Location = New System.Drawing.Point(163, 70)
        Me.ButtonMatrixDown.Name = "ButtonMatrixDown"
        Me.ButtonMatrixDown.Size = New System.Drawing.Size(40, 20)
        Me.ButtonMatrixDown.TabIndex = 22
        Me.ButtonMatrixDown.Text = "▼"
        Me.ButtonMatrixDown.UseVisualStyleBackColor = True
        '
        'LB_Matrices
        '
        Me.LB_Matrices.FormattingEnabled = True
        Me.LB_Matrices.ItemHeight = 16
        Me.LB_Matrices.Location = New System.Drawing.Point(0, 102)
        Me.LB_Matrices.Name = "LB_Matrices"
        Me.LB_Matrices.Size = New System.Drawing.Size(294, 308)
        Me.LB_Matrices.TabIndex = 18
        '
        'ButtonDeleteMatrix
        '
        Me.ButtonDeleteMatrix.Anchor = CType(((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Left) _
            Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.ButtonDeleteMatrix.Enabled = False
        Me.ButtonDeleteMatrix.Image = CType(resources.GetObject("ButtonDeleteMatrix.Image"), System.Drawing.Image)
        Me.ButtonDeleteMatrix.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft
        Me.ButtonDeleteMatrix.Location = New System.Drawing.Point(204, 50)
        Me.ButtonDeleteMatrix.Margin = New System.Windows.Forms.Padding(4)
        Me.ButtonDeleteMatrix.Name = "ButtonDeleteMatrix"
        Me.ButtonDeleteMatrix.Size = New System.Drawing.Size(90, 40)
        Me.ButtonDeleteMatrix.TabIndex = 15
        Me.ButtonDeleteMatrix.Text = "Delete"
        Me.ButtonDeleteMatrix.TextAlign = System.Drawing.ContentAlignment.MiddleRight
        Me.ButtonDeleteMatrix.UseVisualStyleBackColor = True
        '
        'ButtonEditMatrix
        '
        Me.ButtonEditMatrix.Enabled = False
        Me.ButtonEditMatrix.Image = CType(resources.GetObject("ButtonEditMatrix.Image"), System.Drawing.Image)
        Me.ButtonEditMatrix.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft
        Me.ButtonEditMatrix.Location = New System.Drawing.Point(82, 50)
        Me.ButtonEditMatrix.Margin = New System.Windows.Forms.Padding(4)
        Me.ButtonEditMatrix.Name = "ButtonEditMatrix"
        Me.ButtonEditMatrix.Size = New System.Drawing.Size(80, 40)
        Me.ButtonEditMatrix.TabIndex = 13
        Me.ButtonEditMatrix.Text = "Edit"
        Me.ButtonEditMatrix.TextAlign = System.Drawing.ContentAlignment.MiddleRight
        Me.ButtonEditMatrix.UseVisualStyleBackColor = True
        '
        'ButtonNewMatrix
        '
        Me.ButtonNewMatrix.Image = CType(resources.GetObject("ButtonNewMatrix.Image"), System.Drawing.Image)
        Me.ButtonNewMatrix.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft
        Me.ButtonNewMatrix.Location = New System.Drawing.Point(2, 50)
        Me.ButtonNewMatrix.Margin = New System.Windows.Forms.Padding(4)
        Me.ButtonNewMatrix.Name = "ButtonNewMatrix"
        Me.ButtonNewMatrix.Size = New System.Drawing.Size(80, 40)
        Me.ButtonNewMatrix.TabIndex = 14
        Me.ButtonNewMatrix.Text = "New"
        Me.ButtonNewMatrix.TextAlign = System.Drawing.ContentAlignment.MiddleRight
        Me.ButtonNewMatrix.UseVisualStyleBackColor = True
        '
        'Label18
        '
        Me.Label18.Location = New System.Drawing.Point(6, 7)
        Me.Label18.Name = "Label18"
        Me.Label18.Size = New System.Drawing.Size(282, 38)
        Me.Label18.TabIndex = 17
        Me.Label18.Text = "These are region sets in which a top-left region is defined and repeated rectangu" & _
    "larly"
        Me.Label18.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        '
        'Label3
        '
        Me.Label3.BackColor = System.Drawing.Color.Silver
        Me.Label3.Location = New System.Drawing.Point(19, 55)
        Me.Label3.Name = "Label3"
        Me.Label3.Size = New System.Drawing.Size(75, 21)
        Me.Label3.TabIndex = 12
        Me.Label3.Text = "Left"
        Me.Label3.TextAlign = System.Drawing.ContentAlignment.MiddleLeft
        '
        'TB_Right
        '
        Me.TB_Right.Font = New System.Drawing.Font("Microsoft Sans Serif", 12.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.TB_Right.Location = New System.Drawing.Point(100, 82)
        Me.TB_Right.Name = "TB_Right"
        Me.TB_Right.Size = New System.Drawing.Size(80, 26)
        Me.TB_Right.TabIndex = 8
        Me.TB_Right.Text = "0"
        '
        'Label4
        '
        Me.Label4.BackColor = System.Drawing.Color.Silver
        Me.Label4.Location = New System.Drawing.Point(19, 84)
        Me.Label4.Name = "Label4"
        Me.Label4.Size = New System.Drawing.Size(75, 21)
        Me.Label4.TabIndex = 12
        Me.Label4.Text = "Right"
        Me.Label4.TextAlign = System.Drawing.ContentAlignment.MiddleLeft
        '
        'TB_Top
        '
        Me.TB_Top.Font = New System.Drawing.Font("Microsoft Sans Serif", 12.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.TB_Top.Location = New System.Drawing.Point(100, 112)
        Me.TB_Top.Name = "TB_Top"
        Me.TB_Top.Size = New System.Drawing.Size(80, 26)
        Me.TB_Top.TabIndex = 8
        Me.TB_Top.Text = "0"
        '
        'Label5
        '
        Me.Label5.BackColor = System.Drawing.Color.Silver
        Me.Label5.Location = New System.Drawing.Point(19, 114)
        Me.Label5.Name = "Label5"
        Me.Label5.Size = New System.Drawing.Size(75, 21)
        Me.Label5.TabIndex = 12
        Me.Label5.Text = "Top"
        Me.Label5.TextAlign = System.Drawing.ContentAlignment.MiddleLeft
        '
        'Label6
        '
        Me.Label6.BackColor = System.Drawing.Color.Silver
        Me.Label6.Location = New System.Drawing.Point(19, 144)
        Me.Label6.Name = "Label6"
        Me.Label6.Size = New System.Drawing.Size(75, 21)
        Me.Label6.TabIndex = 12
        Me.Label6.Text = "Bottom"
        Me.Label6.TextAlign = System.Drawing.ContentAlignment.MiddleLeft
        '
        'Label7
        '
        Me.Label7.Anchor = CType(((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Left) _
            Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.Label7.BackColor = System.Drawing.Color.Silver
        Me.Label7.Font = New System.Drawing.Font("Microsoft Sans Serif", 14.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label7.Location = New System.Drawing.Point(4, 6)
        Me.Label7.Margin = New System.Windows.Forms.Padding(4, 0, 4, 0)
        Me.Label7.Name = "Label7"
        Me.Label7.Size = New System.Drawing.Size(191, 31)
        Me.Label7.TabIndex = 9
        Me.Label7.Text = "Region limits"
        Me.Label7.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        '
        'TB_Bottom
        '
        Me.TB_Bottom.Font = New System.Drawing.Font("Microsoft Sans Serif", 12.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.TB_Bottom.Location = New System.Drawing.Point(100, 142)
        Me.TB_Bottom.Name = "TB_Bottom"
        Me.TB_Bottom.Size = New System.Drawing.Size(80, 26)
        Me.TB_Bottom.TabIndex = 8
        Me.TB_Bottom.Text = "0"
        '
        'Label10
        '
        Me.Label10.Anchor = CType(((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Left) _
            Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.Label10.BackColor = System.Drawing.Color.Silver
        Me.Label10.Font = New System.Drawing.Font("Microsoft Sans Serif", 14.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label10.Location = New System.Drawing.Point(4, 6)
        Me.Label10.Margin = New System.Windows.Forms.Padding(4, 0, 4, 0)
        Me.Label10.Name = "Label10"
        Me.Label10.Size = New System.Drawing.Size(167, 31)
        Me.Label10.TabIndex = 9
        Me.Label10.Text = "Texture"
        Me.Label10.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        '
        'PanelLimits
        '
        Me.PanelLimits.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle
        Me.PanelLimits.Controls.Add(Me.TB_Left)
        Me.PanelLimits.Controls.Add(Me.TB_Right)
        Me.PanelLimits.Controls.Add(Me.Label6)
        Me.PanelLimits.Controls.Add(Me.TB_Top)
        Me.PanelLimits.Controls.Add(Me.Label7)
        Me.PanelLimits.Controls.Add(Me.TB_Bottom)
        Me.PanelLimits.Controls.Add(Me.Label5)
        Me.PanelLimits.Controls.Add(Me.Label4)
        Me.PanelLimits.Controls.Add(Me.Label3)
        Me.PanelLimits.Enabled = False
        Me.PanelLimits.Location = New System.Drawing.Point(328, 12)
        Me.PanelLimits.Name = "PanelLimits"
        Me.PanelLimits.Size = New System.Drawing.Size(201, 181)
        Me.PanelLimits.TabIndex = 14
        '
        'Panel4
        '
        Me.Panel4.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle
        Me.Panel4.Controls.Add(Me.ButtonColorHotspot)
        Me.Panel4.Controls.Add(Me.ButtonColorRegion)
        Me.Panel4.Controls.Add(Me.LabelZoom)
        Me.Panel4.Controls.Add(Me.Label10)
        Me.Panel4.Controls.Add(Me.ButtonZoomMinus)
        Me.Panel4.Controls.Add(Me.TB_TextureName)
        Me.Panel4.Controls.Add(Me.ButtonZoomPlus)
        Me.Panel4.Controls.Add(Me.ButtonLoadTexture)
        Me.Panel4.Controls.Add(Me.Label25)
        Me.Panel4.Controls.Add(Me.Label11)
        Me.Panel4.Location = New System.Drawing.Point(543, 12)
        Me.Panel4.Name = "Panel4"
        Me.Panel4.Size = New System.Drawing.Size(177, 252)
        Me.Panel4.TabIndex = 15
        '
        'ButtonColorHotspot
        '
        Me.ButtonColorHotspot.BackColor = System.Drawing.Color.Yellow
        Me.ButtonColorHotspot.FlatStyle = System.Windows.Forms.FlatStyle.Flat
        Me.ButtonColorHotspot.Location = New System.Drawing.Point(123, 150)
        Me.ButtonColorHotspot.Name = "ButtonColorHotspot"
        Me.ButtonColorHotspot.Size = New System.Drawing.Size(42, 34)
        Me.ButtonColorHotspot.TabIndex = 14
        Me.ButtonColorHotspot.UseVisualStyleBackColor = False
        '
        'ButtonColorRegion
        '
        Me.ButtonColorRegion.BackColor = System.Drawing.Color.Blue
        Me.ButtonColorRegion.FlatStyle = System.Windows.Forms.FlatStyle.Flat
        Me.ButtonColorRegion.Location = New System.Drawing.Point(72, 150)
        Me.ButtonColorRegion.Name = "ButtonColorRegion"
        Me.ButtonColorRegion.Size = New System.Drawing.Size(42, 34)
        Me.ButtonColorRegion.TabIndex = 14
        Me.ButtonColorRegion.UseVisualStyleBackColor = False
        '
        'LabelZoom
        '
        Me.LabelZoom.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle
        Me.LabelZoom.Font = New System.Drawing.Font("Microsoft Sans Serif", 18.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.LabelZoom.Location = New System.Drawing.Point(8, 197)
        Me.LabelZoom.Name = "LabelZoom"
        Me.LabelZoom.Size = New System.Drawing.Size(46, 45)
        Me.LabelZoom.TabIndex = 13
        Me.LabelZoom.Text = "x1"
        Me.LabelZoom.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        '
        'TB_TextureName
        '
        Me.TB_TextureName.Font = New System.Drawing.Font("Microsoft Sans Serif", 11.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.TB_TextureName.Location = New System.Drawing.Point(8, 115)
        Me.TB_TextureName.Name = "TB_TextureName"
        Me.TB_TextureName.Size = New System.Drawing.Size(159, 24)
        Me.TB_TextureName.TabIndex = 8
        '
        'Label25
        '
        Me.Label25.Location = New System.Drawing.Point(9, 150)
        Me.Label25.Name = "Label25"
        Me.Label25.Size = New System.Drawing.Size(58, 34)
        Me.Label25.TabIndex = 12
        Me.Label25.Text = "Marking colors:"
        Me.Label25.TextAlign = System.Drawing.ContentAlignment.MiddleLeft
        '
        'Label11
        '
        Me.Label11.Location = New System.Drawing.Point(8, 91)
        Me.Label11.Name = "Label11"
        Me.Label11.Size = New System.Drawing.Size(159, 24)
        Me.Label11.TabIndex = 12
        Me.Label11.Text = "Texture name:"
        Me.Label11.TextAlign = System.Drawing.ContentAlignment.MiddleLeft
        '
        'BlinkTimer
        '
        '
        'PanelHotspot
        '
        Me.PanelHotspot.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle
        Me.PanelHotspot.Controls.Add(Me.TB_HotspotX)
        Me.PanelHotspot.Controls.Add(Me.TB_HotspotY)
        Me.PanelHotspot.Controls.Add(Me.Label8)
        Me.PanelHotspot.Controls.Add(Me.Label12)
        Me.PanelHotspot.Controls.Add(Me.Label13)
        Me.PanelHotspot.Enabled = False
        Me.PanelHotspot.Location = New System.Drawing.Point(328, 209)
        Me.PanelHotspot.Name = "PanelHotspot"
        Me.PanelHotspot.Size = New System.Drawing.Size(201, 121)
        Me.PanelHotspot.TabIndex = 14
        '
        'TB_HotspotX
        '
        Me.TB_HotspotX.Font = New System.Drawing.Font("Microsoft Sans Serif", 12.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.TB_HotspotX.Location = New System.Drawing.Point(100, 53)
        Me.TB_HotspotX.Name = "TB_HotspotX"
        Me.TB_HotspotX.Size = New System.Drawing.Size(80, 26)
        Me.TB_HotspotX.TabIndex = 8
        Me.TB_HotspotX.Text = "0"
        '
        'TB_HotspotY
        '
        Me.TB_HotspotY.Font = New System.Drawing.Font("Microsoft Sans Serif", 12.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.TB_HotspotY.Location = New System.Drawing.Point(100, 82)
        Me.TB_HotspotY.Name = "TB_HotspotY"
        Me.TB_HotspotY.Size = New System.Drawing.Size(80, 26)
        Me.TB_HotspotY.TabIndex = 8
        Me.TB_HotspotY.Text = "0"
        '
        'Label8
        '
        Me.Label8.Anchor = CType(((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Left) _
            Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.Label8.BackColor = System.Drawing.Color.Silver
        Me.Label8.Font = New System.Drawing.Font("Microsoft Sans Serif", 14.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label8.Location = New System.Drawing.Point(4, 6)
        Me.Label8.Margin = New System.Windows.Forms.Padding(4, 0, 4, 0)
        Me.Label8.Name = "Label8"
        Me.Label8.Size = New System.Drawing.Size(191, 31)
        Me.Label8.TabIndex = 9
        Me.Label8.Text = "Hotspot"
        Me.Label8.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        '
        'Label12
        '
        Me.Label12.BackColor = System.Drawing.Color.Silver
        Me.Label12.Location = New System.Drawing.Point(19, 84)
        Me.Label12.Name = "Label12"
        Me.Label12.Size = New System.Drawing.Size(75, 21)
        Me.Label12.TabIndex = 12
        Me.Label12.Text = "Hotspot Y"
        Me.Label12.TextAlign = System.Drawing.ContentAlignment.MiddleLeft
        '
        'Label13
        '
        Me.Label13.BackColor = System.Drawing.Color.Silver
        Me.Label13.Location = New System.Drawing.Point(19, 55)
        Me.Label13.Name = "Label13"
        Me.Label13.Size = New System.Drawing.Size(75, 21)
        Me.Label13.TabIndex = 12
        Me.Label13.Text = "Hotspot X"
        Me.Label13.TextAlign = System.Drawing.ContentAlignment.MiddleLeft
        '
        'PanelMatrix
        '
        Me.PanelMatrix.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle
        Me.PanelMatrix.Controls.Add(Me.TB_Rows)
        Me.PanelMatrix.Controls.Add(Me.TB_Separation)
        Me.PanelMatrix.Controls.Add(Me.TB_Columns)
        Me.PanelMatrix.Controls.Add(Me.Label15)
        Me.PanelMatrix.Controls.Add(Me.Label2)
        Me.PanelMatrix.Controls.Add(Me.Label9)
        Me.PanelMatrix.Controls.Add(Me.Label14)
        Me.PanelMatrix.Enabled = False
        Me.PanelMatrix.Location = New System.Drawing.Point(328, 345)
        Me.PanelMatrix.Name = "PanelMatrix"
        Me.PanelMatrix.Size = New System.Drawing.Size(201, 150)
        Me.PanelMatrix.TabIndex = 14
        '
        'TB_Rows
        '
        Me.TB_Rows.Font = New System.Drawing.Font("Microsoft Sans Serif", 12.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.TB_Rows.Location = New System.Drawing.Point(100, 82)
        Me.TB_Rows.Name = "TB_Rows"
        Me.TB_Rows.Size = New System.Drawing.Size(80, 26)
        Me.TB_Rows.TabIndex = 8
        Me.TB_Rows.Text = "1"
        '
        'TB_Separation
        '
        Me.TB_Separation.Font = New System.Drawing.Font("Microsoft Sans Serif", 12.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.TB_Separation.Location = New System.Drawing.Point(100, 111)
        Me.TB_Separation.Name = "TB_Separation"
        Me.TB_Separation.Size = New System.Drawing.Size(80, 26)
        Me.TB_Separation.TabIndex = 8
        Me.TB_Separation.Text = "0"
        '
        'TB_Columns
        '
        Me.TB_Columns.Font = New System.Drawing.Font("Microsoft Sans Serif", 12.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.TB_Columns.Location = New System.Drawing.Point(100, 53)
        Me.TB_Columns.Name = "TB_Columns"
        Me.TB_Columns.Size = New System.Drawing.Size(80, 26)
        Me.TB_Columns.TabIndex = 8
        Me.TB_Columns.Text = "1"
        '
        'Label15
        '
        Me.Label15.BackColor = System.Drawing.Color.Silver
        Me.Label15.Location = New System.Drawing.Point(19, 113)
        Me.Label15.Name = "Label15"
        Me.Label15.Size = New System.Drawing.Size(75, 21)
        Me.Label15.TabIndex = 12
        Me.Label15.Text = "Separation"
        Me.Label15.TextAlign = System.Drawing.ContentAlignment.MiddleLeft
        '
        'Label2
        '
        Me.Label2.Anchor = CType(((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Left) _
            Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.Label2.BackColor = System.Drawing.Color.Silver
        Me.Label2.Font = New System.Drawing.Font("Microsoft Sans Serif", 14.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label2.Location = New System.Drawing.Point(4, 6)
        Me.Label2.Margin = New System.Windows.Forms.Padding(4, 0, 4, 0)
        Me.Label2.Name = "Label2"
        Me.Label2.Size = New System.Drawing.Size(191, 31)
        Me.Label2.TabIndex = 9
        Me.Label2.Text = "Matrix size"
        Me.Label2.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        '
        'Label9
        '
        Me.Label9.BackColor = System.Drawing.Color.Silver
        Me.Label9.Location = New System.Drawing.Point(19, 55)
        Me.Label9.Name = "Label9"
        Me.Label9.Size = New System.Drawing.Size(75, 21)
        Me.Label9.TabIndex = 12
        Me.Label9.Text = "Columns"
        Me.Label9.TextAlign = System.Drawing.ContentAlignment.MiddleLeft
        '
        'Label14
        '
        Me.Label14.BackColor = System.Drawing.Color.Silver
        Me.Label14.Location = New System.Drawing.Point(19, 84)
        Me.Label14.Name = "Label14"
        Me.Label14.Size = New System.Drawing.Size(75, 21)
        Me.Label14.TabIndex = 12
        Me.Label14.Text = "Rows"
        Me.Label14.TextAlign = System.Drawing.ContentAlignment.MiddleLeft
        '
        'PanelProject
        '
        Me.PanelProject.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle
        Me.PanelProject.Controls.Add(Me.ButtonExportASM)
        Me.PanelProject.Controls.Add(Me.ButtonExportHeader)
        Me.PanelProject.Controls.Add(Me.ButtonSaveProject)
        Me.PanelProject.Controls.Add(Me.ButtonLoadProject)
        Me.PanelProject.Controls.Add(Me.Label16)
        Me.PanelProject.Location = New System.Drawing.Point(543, 279)
        Me.PanelProject.Name = "PanelProject"
        Me.PanelProject.Size = New System.Drawing.Size(177, 217)
        Me.PanelProject.TabIndex = 14
        '
        'ButtonExportHeader
        '
        Me.ButtonExportHeader.Enabled = False
        Me.ButtonExportHeader.Image = CType(resources.GetObject("ButtonExportHeader.Image"), System.Drawing.Image)
        Me.ButtonExportHeader.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft
        Me.ButtonExportHeader.Location = New System.Drawing.Point(8, 127)
        Me.ButtonExportHeader.Name = "ButtonExportHeader"
        Me.ButtonExportHeader.Size = New System.Drawing.Size(159, 38)
        Me.ButtonExportHeader.TabIndex = 10
        Me.ButtonExportHeader.Text = "Export C header"
        Me.ButtonExportHeader.TextAlign = System.Drawing.ContentAlignment.MiddleRight
        Me.ButtonExportHeader.UseVisualStyleBackColor = True
        '
        'ButtonSaveProject
        '
        Me.ButtonSaveProject.Enabled = False
        Me.ButtonSaveProject.Image = CType(resources.GetObject("ButtonSaveProject.Image"), System.Drawing.Image)
        Me.ButtonSaveProject.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft
        Me.ButtonSaveProject.Location = New System.Drawing.Point(8, 85)
        Me.ButtonSaveProject.Name = "ButtonSaveProject"
        Me.ButtonSaveProject.Size = New System.Drawing.Size(159, 38)
        Me.ButtonSaveProject.TabIndex = 11
        Me.ButtonSaveProject.Text = "Save project XML"
        Me.ButtonSaveProject.TextAlign = System.Drawing.ContentAlignment.MiddleRight
        Me.ButtonSaveProject.UseVisualStyleBackColor = True
        '
        'ButtonLoadProject
        '
        Me.ButtonLoadProject.Image = CType(resources.GetObject("ButtonLoadProject.Image"), System.Drawing.Image)
        Me.ButtonLoadProject.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft
        Me.ButtonLoadProject.Location = New System.Drawing.Point(8, 43)
        Me.ButtonLoadProject.Name = "ButtonLoadProject"
        Me.ButtonLoadProject.Size = New System.Drawing.Size(159, 38)
        Me.ButtonLoadProject.TabIndex = 12
        Me.ButtonLoadProject.Text = "Load project XML"
        Me.ButtonLoadProject.TextAlign = System.Drawing.ContentAlignment.MiddleRight
        Me.ButtonLoadProject.UseVisualStyleBackColor = True
        '
        'Label16
        '
        Me.Label16.Anchor = CType(((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Left) _
            Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.Label16.BackColor = System.Drawing.Color.Silver
        Me.Label16.Font = New System.Drawing.Font("Microsoft Sans Serif", 14.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label16.Location = New System.Drawing.Point(4, 6)
        Me.Label16.Margin = New System.Windows.Forms.Padding(4, 0, 4, 0)
        Me.Label16.Name = "Label16"
        Me.Label16.Size = New System.Drawing.Size(167, 31)
        Me.Label16.TabIndex = 9
        Me.Label16.Text = "Project"
        Me.Label16.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        '
        'PanelTools
        '
        Me.PanelTools.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle
        Me.PanelTools.Controls.Add(Me.Label24)
        Me.PanelTools.Controls.Add(Me.Label23)
        Me.PanelTools.Controls.Add(Me.Label22)
        Me.PanelTools.Controls.Add(Me.Label21)
        Me.PanelTools.Controls.Add(Me.Label19)
        Me.PanelTools.Controls.Add(Me.RB_Pan)
        Me.PanelTools.Controls.Add(Me.RB_MoveRegion)
        Me.PanelTools.Controls.Add(Me.RB_Hotspot)
        Me.PanelTools.Controls.Add(Me.RB_BottomRight)
        Me.PanelTools.Controls.Add(Me.RB_TopLeft)
        Me.PanelTools.Controls.Add(Me.Label20)
        Me.PanelTools.Enabled = False
        Me.PanelTools.Location = New System.Drawing.Point(734, 13)
        Me.PanelTools.Name = "PanelTools"
        Me.PanelTools.Size = New System.Drawing.Size(75, 483)
        Me.PanelTools.TabIndex = 15
        '
        'Label24
        '
        Me.Label24.Font = New System.Drawing.Font("Microsoft Sans Serif", 9.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label24.Location = New System.Drawing.Point(3, 395)
        Me.Label24.Name = "Label24"
        Me.Label24.Size = New System.Drawing.Size(65, 16)
        Me.Label24.TabIndex = 11
        Me.Label24.Text = "Move all"
        Me.Label24.TextAlign = System.Drawing.ContentAlignment.BottomCenter
        '
        'Label23
        '
        Me.Label23.Font = New System.Drawing.Font("Microsoft Sans Serif", 9.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label23.Location = New System.Drawing.Point(3, 305)
        Me.Label23.Name = "Label23"
        Me.Label23.Size = New System.Drawing.Size(65, 16)
        Me.Label23.TabIndex = 11
        Me.Label23.Text = "Hotspot"
        Me.Label23.TextAlign = System.Drawing.ContentAlignment.BottomCenter
        '
        'Label22
        '
        Me.Label22.Font = New System.Drawing.Font("Microsoft Sans Serif", 9.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label22.Location = New System.Drawing.Point(3, 220)
        Me.Label22.Name = "Label22"
        Me.Label22.Size = New System.Drawing.Size(65, 16)
        Me.Label22.TabIndex = 11
        Me.Label22.Text = "Bott-right"
        Me.Label22.TextAlign = System.Drawing.ContentAlignment.BottomCenter
        '
        'Label21
        '
        Me.Label21.Font = New System.Drawing.Font("Microsoft Sans Serif", 9.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label21.Location = New System.Drawing.Point(4, 135)
        Me.Label21.Name = "Label21"
        Me.Label21.Size = New System.Drawing.Size(65, 16)
        Me.Label21.TabIndex = 11
        Me.Label21.Text = "Top-left"
        Me.Label21.TextAlign = System.Drawing.ContentAlignment.BottomCenter
        '
        'Label19
        '
        Me.Label19.Font = New System.Drawing.Font("Microsoft Sans Serif", 9.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label19.Location = New System.Drawing.Point(4, 50)
        Me.Label19.Name = "Label19"
        Me.Label19.Size = New System.Drawing.Size(65, 16)
        Me.Label19.TabIndex = 11
        Me.Label19.Text = "Pan view"
        Me.Label19.TextAlign = System.Drawing.ContentAlignment.BottomCenter
        '
        'RB_Pan
        '
        Me.RB_Pan.Appearance = System.Windows.Forms.Appearance.Button
        Me.RB_Pan.BackColor = System.Drawing.Color.RosyBrown
        Me.RB_Pan.Checked = True
        Me.RB_Pan.Image = CType(resources.GetObject("RB_Pan.Image"), System.Drawing.Image)
        Me.RB_Pan.Location = New System.Drawing.Point(11, 71)
        Me.RB_Pan.Name = "RB_Pan"
        Me.RB_Pan.Size = New System.Drawing.Size(50, 50)
        Me.RB_Pan.TabIndex = 10
        Me.RB_Pan.TabStop = True
        Me.TTHelp.SetToolTip(Me.RB_Pan, "Left click on image drags the image view")
        Me.RB_Pan.UseVisualStyleBackColor = False
        '
        'RB_MoveRegion
        '
        Me.RB_MoveRegion.Appearance = System.Windows.Forms.Appearance.Button
        Me.RB_MoveRegion.BackColor = System.Drawing.Color.CadetBlue
        Me.RB_MoveRegion.Image = CType(resources.GetObject("RB_MoveRegion.Image"), System.Drawing.Image)
        Me.RB_MoveRegion.Location = New System.Drawing.Point(11, 416)
        Me.RB_MoveRegion.Name = "RB_MoveRegion"
        Me.RB_MoveRegion.Size = New System.Drawing.Size(50, 50)
        Me.RB_MoveRegion.TabIndex = 10
        Me.TTHelp.SetToolTip(Me.RB_MoveRegion, "Left click on image drags the region")
        Me.RB_MoveRegion.UseVisualStyleBackColor = False
        '
        'RB_Hotspot
        '
        Me.RB_Hotspot.Appearance = System.Windows.Forms.Appearance.Button
        Me.RB_Hotspot.BackColor = System.Drawing.Color.CadetBlue
        Me.RB_Hotspot.Image = CType(resources.GetObject("RB_Hotspot.Image"), System.Drawing.Image)
        Me.RB_Hotspot.Location = New System.Drawing.Point(11, 326)
        Me.RB_Hotspot.Name = "RB_Hotspot"
        Me.RB_Hotspot.Size = New System.Drawing.Size(50, 50)
        Me.RB_Hotspot.TabIndex = 10
        Me.TTHelp.SetToolTip(Me.RB_Hotspot, "Left click on image sets hotspot")
        Me.RB_Hotspot.UseVisualStyleBackColor = False
        '
        'RB_BottomRight
        '
        Me.RB_BottomRight.Appearance = System.Windows.Forms.Appearance.Button
        Me.RB_BottomRight.BackColor = System.Drawing.Color.CadetBlue
        Me.RB_BottomRight.Image = CType(resources.GetObject("RB_BottomRight.Image"), System.Drawing.Image)
        Me.RB_BottomRight.Location = New System.Drawing.Point(11, 241)
        Me.RB_BottomRight.Name = "RB_BottomRight"
        Me.RB_BottomRight.Size = New System.Drawing.Size(50, 50)
        Me.RB_BottomRight.TabIndex = 10
        Me.TTHelp.SetToolTip(Me.RB_BottomRight, "Left click on image sets bottom-right corner")
        Me.RB_BottomRight.UseVisualStyleBackColor = False
        '
        'RB_TopLeft
        '
        Me.RB_TopLeft.Appearance = System.Windows.Forms.Appearance.Button
        Me.RB_TopLeft.BackColor = System.Drawing.Color.CadetBlue
        Me.RB_TopLeft.Image = CType(resources.GetObject("RB_TopLeft.Image"), System.Drawing.Image)
        Me.RB_TopLeft.Location = New System.Drawing.Point(11, 156)
        Me.RB_TopLeft.Name = "RB_TopLeft"
        Me.RB_TopLeft.Size = New System.Drawing.Size(50, 50)
        Me.RB_TopLeft.TabIndex = 10
        Me.TTHelp.SetToolTip(Me.RB_TopLeft, "Left click on image sets top-left corner")
        Me.RB_TopLeft.UseVisualStyleBackColor = False
        '
        'Label20
        '
        Me.Label20.Anchor = CType(((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Left) _
            Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.Label20.BackColor = System.Drawing.Color.Silver
        Me.Label20.Font = New System.Drawing.Font("Microsoft Sans Serif", 14.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label20.Location = New System.Drawing.Point(4, 6)
        Me.Label20.Margin = New System.Windows.Forms.Padding(4, 0, 4, 0)
        Me.Label20.Name = "Label20"
        Me.Label20.Size = New System.Drawing.Size(65, 31)
        Me.Label20.TabIndex = 9
        Me.Label20.Text = "Tools"
        Me.Label20.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        '
        'ButtonExportASM
        '
        Me.ButtonExportASM.Enabled = False
        Me.ButtonExportASM.Font = New System.Drawing.Font("Microsoft Sans Serif", 9.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.ButtonExportASM.Image = CType(resources.GetObject("ButtonExportASM.Image"), System.Drawing.Image)
        Me.ButtonExportASM.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft
        Me.ButtonExportASM.Location = New System.Drawing.Point(8, 169)
        Me.ButtonExportASM.Name = "ButtonExportASM"
        Me.ButtonExportASM.Size = New System.Drawing.Size(159, 38)
        Me.ButtonExportASM.TabIndex = 13
        Me.ButtonExportASM.Text = "Export ASM header"
        Me.ButtonExportASM.TextAlign = System.Drawing.ContentAlignment.MiddleRight
        Me.ButtonExportASM.UseVisualStyleBackColor = True
        '
        'FormEditor
        '
        Me.AutoScaleDimensions = New System.Drawing.SizeF(8.0!, 16.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.ClientSize = New System.Drawing.Size(1322, 507)
        Me.Controls.Add(Me.PanelTools)
        Me.Controls.Add(Me.Panel4)
        Me.Controls.Add(Me.PanelMatrix)
        Me.Controls.Add(Me.PanelProject)
        Me.Controls.Add(Me.PanelHotspot)
        Me.Controls.Add(Me.PanelLimits)
        Me.Controls.Add(Me.PanelRegionLists)
        Me.Controls.Add(Me.PanelTexture)
        Me.Font = New System.Drawing.Font("Microsoft Sans Serif", 9.75!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Margin = New System.Windows.Forms.Padding(4)
        Me.MinimumSize = New System.Drawing.Size(1200, 534)
        Me.Name = "FormEditor"
        Me.Text = "Vircon32 texture regions editor"
        Me.PanelTexture.ResumeLayout(False)
        CType(Me.PictureTexture, System.ComponentModel.ISupportInitialize).EndInit()
        Me.PanelRegionLists.ResumeLayout(False)
        Me.TabRegionTypes.ResumeLayout(False)
        Me.TabRegions.ResumeLayout(False)
        Me.TabMatrices.ResumeLayout(False)
        Me.PanelLimits.ResumeLayout(False)
        Me.PanelLimits.PerformLayout()
        Me.Panel4.ResumeLayout(False)
        Me.Panel4.PerformLayout()
        Me.PanelHotspot.ResumeLayout(False)
        Me.PanelHotspot.PerformLayout()
        Me.PanelMatrix.ResumeLayout(False)
        Me.PanelMatrix.PerformLayout()
        Me.PanelProject.ResumeLayout(False)
        Me.PanelTools.ResumeLayout(False)
        Me.ResumeLayout(False)

    End Sub
    Friend WithEvents PictureTexture As CustomPictureBox
    Friend WithEvents Label1 As System.Windows.Forms.Label
    Friend WithEvents ButtonDeleteRegion As System.Windows.Forms.Button
    Friend WithEvents ButtonNewRegion As System.Windows.Forms.Button
    Friend WithEvents TB_Left As System.Windows.Forms.TextBox
    Friend WithEvents ButtonLoadTexture As System.Windows.Forms.Button
    Friend WithEvents ButtonZoomMinus As System.Windows.Forms.Button
    Friend WithEvents ButtonZoomPlus As System.Windows.Forms.Button
    Friend WithEvents OpenDialog As System.Windows.Forms.OpenFileDialog
    Friend WithEvents PanelTexture As System.Windows.Forms.Panel
    Friend WithEvents PanelRegionLists As System.Windows.Forms.Panel
    Friend WithEvents TabRegionTypes As System.Windows.Forms.TabControl
    Friend WithEvents TabRegions As System.Windows.Forms.TabPage
    Friend WithEvents TabMatrices As System.Windows.Forms.TabPage
    Friend WithEvents Label3 As System.Windows.Forms.Label
    Friend WithEvents TB_Right As System.Windows.Forms.TextBox
    Friend WithEvents Label4 As System.Windows.Forms.Label
    Friend WithEvents TB_Top As System.Windows.Forms.TextBox
    Friend WithEvents Label5 As System.Windows.Forms.Label
    Friend WithEvents Label6 As System.Windows.Forms.Label
    Friend WithEvents Label7 As System.Windows.Forms.Label
    Friend WithEvents TB_Bottom As System.Windows.Forms.TextBox
    Friend WithEvents Label10 As System.Windows.Forms.Label
    Friend WithEvents PanelLimits As System.Windows.Forms.Panel
    Friend WithEvents Panel4 As System.Windows.Forms.Panel
    Friend WithEvents ButtonEditRegion As System.Windows.Forms.Button
    Friend WithEvents TB_TextureName As System.Windows.Forms.TextBox
    Friend WithEvents Label11 As System.Windows.Forms.Label
    Friend WithEvents BlinkTimer As System.Windows.Forms.Timer
    Friend WithEvents PanelHotspot As System.Windows.Forms.Panel
    Friend WithEvents TB_HotspotX As System.Windows.Forms.TextBox
    Friend WithEvents TB_HotspotY As System.Windows.Forms.TextBox
    Friend WithEvents Label8 As System.Windows.Forms.Label
    Friend WithEvents Label12 As System.Windows.Forms.Label
    Friend WithEvents Label13 As System.Windows.Forms.Label
    Friend WithEvents PanelMatrix As System.Windows.Forms.Panel
    Friend WithEvents TB_Rows As System.Windows.Forms.TextBox
    Friend WithEvents TB_Separation As System.Windows.Forms.TextBox
    Friend WithEvents TB_Columns As System.Windows.Forms.TextBox
    Friend WithEvents Label15 As System.Windows.Forms.Label
    Friend WithEvents Label2 As System.Windows.Forms.Label
    Friend WithEvents Label9 As System.Windows.Forms.Label
    Friend WithEvents Label14 As System.Windows.Forms.Label
    Friend WithEvents PanelProject As System.Windows.Forms.Panel
    Friend WithEvents ButtonExportHeader As System.Windows.Forms.Button
    Friend WithEvents ButtonSaveProject As System.Windows.Forms.Button
    Friend WithEvents ButtonLoadProject As System.Windows.Forms.Button
    Friend WithEvents Label16 As System.Windows.Forms.Label
    Friend WithEvents Label17 As System.Windows.Forms.Label
    Friend WithEvents ButtonDeleteMatrix As System.Windows.Forms.Button
    Friend WithEvents ButtonEditMatrix As System.Windows.Forms.Button
    Friend WithEvents ButtonNewMatrix As System.Windows.Forms.Button
    Friend WithEvents Label18 As System.Windows.Forms.Label
    Friend WithEvents PanelTools As System.Windows.Forms.Panel
    Friend WithEvents Label20 As System.Windows.Forms.Label
    Friend WithEvents RB_MoveRegion As System.Windows.Forms.RadioButton
    Friend WithEvents RB_Hotspot As System.Windows.Forms.RadioButton
    Friend WithEvents RB_BottomRight As System.Windows.Forms.RadioButton
    Friend WithEvents RB_TopLeft As System.Windows.Forms.RadioButton
    Friend WithEvents TTHelp As System.Windows.Forms.ToolTip
    Friend WithEvents Label24 As System.Windows.Forms.Label
    Friend WithEvents Label23 As System.Windows.Forms.Label
    Friend WithEvents Label22 As System.Windows.Forms.Label
    Friend WithEvents Label21 As System.Windows.Forms.Label
    Friend WithEvents Label19 As System.Windows.Forms.Label
    Friend WithEvents RB_Pan As System.Windows.Forms.RadioButton
    Friend WithEvents LabelZoom As System.Windows.Forms.Label
    Friend WithEvents LB_Regions As System.Windows.Forms.ListBox
    Friend WithEvents LB_Matrices As System.Windows.Forms.ListBox
    Friend WithEvents SaveDialog As System.Windows.Forms.SaveFileDialog
    Friend WithEvents ButtonRegionUp As System.Windows.Forms.Button
    Friend WithEvents ButtonRegionDown As System.Windows.Forms.Button
    Friend WithEvents ButtonMatrixUp As System.Windows.Forms.Button
    Friend WithEvents ButtonMatrixDown As System.Windows.Forms.Button
    Friend WithEvents ButtonColorHotspot As System.Windows.Forms.Button
    Friend WithEvents ButtonColorRegion As System.Windows.Forms.Button
    Friend WithEvents Label25 As System.Windows.Forms.Label
    Friend WithEvents ColorPicker As System.Windows.Forms.ColorDialog
    Friend WithEvents ButtonExportASM As System.Windows.Forms.Button

End Class
