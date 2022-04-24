Public Class FormEditor

    ' ----------------------------------------
    '   SUPPORT FOR DRAG AND DROP IN TEXTURE
    ' ----------------------------------------
    Public Dragging As Boolean = False
    Public InitialDragPoint As Point
    Public InitialScrollX As Integer
    Public InitialScrollY As Integer
    Public LastDragPixelX As Integer
    Public LastDragPixelY As Integer

    Private Sub FormEditor_Load(sender As Object, e As EventArgs) Handles MyBase.Load
        MainForm = Me
        BlinkTimer.Start()

        LB_Regions.DataSource = DefinedRegions
        LB_Regions.DisplayMember = "GetName"

        LB_Matrices.DataSource = DefinedMatrices
        LB_Matrices.DisplayMember = "GetName"

    End Sub

    Private Sub PictureTexture_MouseDown(sender As Object, e As MouseEventArgs) Handles PictureTexture.MouseDown

        If e.Button <> Windows.Forms.MouseButtons.Left Then Return
        Dragging = True

        ' determine the mouse's current pixel on the texture
        Dim MousePixelX = PictureTexture.PointToClient(MousePosition).X / PictureTexture.ZoomLevel
        Dim MousePixelY = PictureTexture.PointToClient(MousePosition).Y / PictureTexture.ZoomLevel

        ' select the current tool
        If RB_Pan.Checked Then

            InitialDragPoint = MousePosition
            InitialScrollX = PanelTexture.HorizontalScroll.Value
            InitialScrollY = PanelTexture.VerticalScroll.Value

        ElseIf RB_TopLeft.Checked Then

            If ActiveRegion Is Nothing Then Return
            ActiveRegion.Left = MousePixelX
            ActiveRegion.Top = MousePixelY

            ' adjust the opposite corner if needed
            If ActiveRegion.Right < ActiveRegion.Left Then ActiveRegion.Right = ActiveRegion.Left
            If ActiveRegion.Bottom < ActiveRegion.Top Then ActiveRegion.Bottom = ActiveRegion.Top

        ElseIf RB_BottomRight.Checked Then

            If ActiveRegion Is Nothing Then Return
            ActiveRegion.Right = MousePixelX
            ActiveRegion.Bottom = MousePixelY

            ' adjust the opposite corner if needed
            If ActiveRegion.Left > ActiveRegion.Right Then ActiveRegion.Left = ActiveRegion.Right
            If ActiveRegion.Top > ActiveRegion.Bottom Then ActiveRegion.Top = ActiveRegion.Bottom

        ElseIf RB_Hotspot.Checked Then

            If ActiveRegion Is Nothing Then Return
            ActiveRegion.HotspotX = MousePixelX
            ActiveRegion.HotspotY = MousePixelY

        ElseIf RB_MoveRegion.Checked Then

            LastDragPixelX = MousePixelX
            LastDragPixelY = MousePixelY

        End If

        ' in any case, force to show updates
        Invalidate()
        ReadRegionProperties()

    End Sub

    Private Sub PictureTexture_MouseMove(sender As Object, e As EventArgs) Handles PictureTexture.MouseMove

        If Not Dragging Then Return

        ' determine the mouse's current pixel on the texture
        Dim MousePixelX = PictureTexture.PointToClient(MousePosition).X / PictureTexture.ZoomLevel
        Dim MousePixelY = PictureTexture.PointToClient(MousePosition).Y / PictureTexture.ZoomLevel

        ' select the current tool
        If RB_Pan.Checked Then

            Dim NewScrollX = InitialScrollX - (MousePosition.X - InitialDragPoint.X)
            Dim NewScrollY = InitialScrollY - (MousePosition.Y - InitialDragPoint.Y)

            ' clamp to correct values
            If (NewScrollX < PanelTexture.HorizontalScroll.Minimum) Then NewScrollX = PanelTexture.HorizontalScroll.Minimum
            If (NewScrollX > PanelTexture.HorizontalScroll.Maximum) Then NewScrollX = PanelTexture.HorizontalScroll.Maximum
            If (NewScrollY < PanelTexture.VerticalScroll.Minimum) Then NewScrollY = PanelTexture.VerticalScroll.Minimum
            If (NewScrollY > PanelTexture.VerticalScroll.Maximum) Then NewScrollY = PanelTexture.VerticalScroll.Maximum

            PanelTexture.HorizontalScroll.Value = NewScrollX
            PanelTexture.VerticalScroll.Value = NewScrollY

        ElseIf RB_TopLeft.Checked Then

            If ActiveRegion Is Nothing Then Return
            ActiveRegion.Left = MousePixelX
            ActiveRegion.Top = MousePixelY

            ' adjust the opposite corner if needed
            If ActiveRegion.Right < ActiveRegion.Left Then ActiveRegion.Right = ActiveRegion.Left
            If ActiveRegion.Bottom < ActiveRegion.Top Then ActiveRegion.Bottom = ActiveRegion.Top

        ElseIf RB_BottomRight.Checked Then

            If ActiveRegion Is Nothing Then Return
            ActiveRegion.Right = MousePixelX
            ActiveRegion.Bottom = MousePixelY

            ' adjust the opposite corner if needed
            If ActiveRegion.Left > ActiveRegion.Right Then ActiveRegion.Left = ActiveRegion.Right
            If ActiveRegion.Top > ActiveRegion.Bottom Then ActiveRegion.Top = ActiveRegion.Bottom

        ElseIf RB_Hotspot.Checked Then

            If ActiveRegion Is Nothing Then Return
            ActiveRegion.HotspotX = MousePixelX
            ActiveRegion.HotspotY = MousePixelY

        ElseIf RB_MoveRegion.Checked Then

            If ActiveRegion Is Nothing Then Return

            ActiveRegion.MoveRegion(MousePixelX - LastDragPixelX, MousePixelY - LastDragPixelY, LoadedTexture.Width, LoadedTexture.Height)

            LastDragPixelX = MousePixelX
            LastDragPixelY = MousePixelY

        End If

        ' in any case, force to show updates
        Invalidate()
        ReadRegionProperties()

    End Sub

    Private Sub PictureTexture_MouseUp(sender As Object, e As MouseEventArgs) Handles PictureTexture.MouseUp
        If e.Button = Windows.Forms.MouseButtons.Left Then
            Dragging = False
        End If
    End Sub

    ' ----------------------------------------
    '   SUPPORT FOR TEXTURE ZOOM AND SCROLL
    ' ----------------------------------------

    Private Sub ButtonZoomPlus_Click(sender As Object, e As EventArgs) Handles ButtonZoomPlus.Click

        PictureTexture.ZoomLevel += 1
        LabelZoom.Text = "x" & PictureTexture.ZoomLevel

    End Sub

    Private Sub ButtonZoomMinus_Click(sender As Object, e As EventArgs) Handles ButtonZoomMinus.Click

        PictureTexture.ZoomLevel -= 1
        LabelZoom.Text = "x" & PictureTexture.ZoomLevel

    End Sub

    Private Sub PictureTexture_SizeChanged(sender As Object, e As EventArgs) Handles PictureTexture.SizeChanged
        AdjustPictureScroll()
    End Sub

    Private Sub PanelTexture_SizeChanged(sender As Object, e As EventArgs) Handles PanelTexture.SizeChanged
        AdjustPictureScroll()
    End Sub

    Public Sub AdjustPictureScroll()
        PanelTexture.HorizontalScroll.Maximum = PictureTexture.Width - PanelTexture.Width - 1
        PanelTexture.VerticalScroll.Maximum = PictureTexture.Height - PanelTexture.Height - 1
    End Sub

    ' ----------------------------------------
    '   PROCESSING OTHER EVENTS
    ' ----------------------------------------

    Private Sub BlinkTimer_Tick(sender As Object, e As EventArgs) Handles BlinkTimer.Tick
        PictureTexture.DashOffset += 1
        PictureTexture.ShowHotspot = Not PictureTexture.ShowHotspot
        PictureTexture.Refresh()
    End Sub

    Private Sub RBTools_CheckedChanged(sender As Object, e As EventArgs) Handles _
        RB_TopLeft.CheckedChanged, RB_BottomRight.CheckedChanged, RB_Hotspot.CheckedChanged, RB_MoveRegion.CheckedChanged, RB_Pan.CheckedChanged

        RB_TopLeft.BackColor = If(RB_TopLeft.Checked, Color.RosyBrown, Color.CadetBlue)
        RB_BottomRight.BackColor = If(RB_BottomRight.Checked, Color.RosyBrown, Color.CadetBlue)
        RB_Hotspot.BackColor = If(RB_Hotspot.Checked, Color.RosyBrown, Color.CadetBlue)
        RB_MoveRegion.BackColor = If(RB_MoveRegion.Checked, Color.RosyBrown, Color.CadetBlue)
        RB_Pan.BackColor = If(RB_Pan.Checked, Color.RosyBrown, Color.CadetBlue)

    End Sub

    Private Sub ButtonColorRegion_Click(sender As Object, e As EventArgs) Handles ButtonColorRegion.Click

        ColorPicker.Color = ButtonColorRegion.BackColor

        If ColorPicker.ShowDialog(Me) = Windows.Forms.DialogResult.OK Then
            ButtonColorRegion.BackColor = ColorPicker.Color
        End If

    End Sub

    Private Sub ButtonColorHotspot_Click(sender As Object, e As EventArgs) Handles ButtonColorHotspot.Click

        ColorPicker.Color = ButtonColorHotspot.BackColor

        If ColorPicker.ShowDialog(Me) = Windows.Forms.DialogResult.OK Then
            ButtonColorHotspot.BackColor = ColorPicker.Color
        End If

    End Sub

    ' ----------------------------------------
    '   PROCESSING MANUAL TEXT/NUMBER INPUTS
    ' ----------------------------------------

    Private Sub TB_TextureName_Validating(sender As Object, e As System.ComponentModel.CancelEventArgs) Handles TB_TextureName.Validating
        TB_TextureName.Text = MakeIdentifier(TB_TextureName.Text)
        TextureName = TB_TextureName.Text
    End Sub

    Private Sub TB_Left_Validating(sender As Object, e As System.ComponentModel.CancelEventArgs) Handles TB_Left.Validating
        WriteXCoordinate(ActiveRegion.Left, TB_Left.Text, True)
    End Sub

    Private Sub TB_Right_Validating(sender As Object, e As System.ComponentModel.CancelEventArgs) Handles TB_Right.Validating
        WriteXCoordinate(ActiveRegion.Right, TB_Right.Text, True)
    End Sub

    Private Sub TB_Top_Validating(sender As Object, e As System.ComponentModel.CancelEventArgs) Handles TB_Top.Validating
        WriteYCoordinate(ActiveRegion.Top, TB_Top.Text, True)
    End Sub

    Private Sub TB_Bottom_Validating(sender As Object, e As System.ComponentModel.CancelEventArgs) Handles TB_Bottom.Validating
        WriteYCoordinate(ActiveRegion.Bottom, TB_Bottom.Text, True)
    End Sub

    Private Sub TB_HotspotX_Validating(sender As Object, e As System.ComponentModel.CancelEventArgs) Handles TB_HotspotX.Validating
        WriteXCoordinate(ActiveRegion.HotspotX, TB_HotspotX.Text, False)
    End Sub

    Private Sub TB_HotspotY_Validating(sender As Object, e As System.ComponentModel.CancelEventArgs) Handles TB_HotspotY.Validating
        WriteYCoordinate(ActiveRegion.HotspotY, TB_HotspotY.Text, False)
    End Sub

    Private Sub TB_Separation_Validating(sender As Object, e As System.ComponentModel.CancelEventArgs) Handles TB_Separation.Validating
        ' not a coordinate but we can apply the same logic with no clamping
        WriteXCoordinate(ActiveMatrix.Separation, TB_Separation.Text, False)
    End Sub

    Private Sub TB_Columns_Validating(sender As Object, e As System.ComponentModel.CancelEventArgs) Handles TB_Columns.Validating
        WriteMatrixDimension(ActiveMatrix.Columns, TB_Columns.Text)
    End Sub

    Private Sub TB_Rows_Validating(sender As Object, e As System.ComponentModel.CancelEventArgs) Handles TB_Rows.Validating
        WriteMatrixDimension(ActiveMatrix.Rows, TB_Rows.Text)
    End Sub

    ' --------------------------------
    '   HANDLING THE LIST OF REGIONS
    ' --------------------------------

    Public Sub ForceListUpdates()

        LB_Regions.DataSource = Nothing
        LB_Regions.DataSource = DefinedRegions
        LB_Regions.DisplayMember = "GetName"

        LB_Matrices.DataSource = Nothing
        LB_Matrices.DataSource = DefinedMatrices
        LB_Matrices.DisplayMember = "GetName"

    End Sub

    Private Sub LB_Regions_SelectedIndexChanged(sender As Object, e As EventArgs) Handles LB_Regions.SelectedIndexChanged

        ActiveRegion = LB_Regions.SelectedValue
        ActiveMatrix = Nothing
        ReadRegionProperties()
        ReadMatrixProperties()

        ' update list handling buttons
        Dim SelectionNotEmpty = (LB_Regions.SelectedIndex >= 0 AndAlso Not ActiveRegion Is Nothing)
        ButtonEditRegion.Enabled = SelectionNotEmpty
        ButtonDeleteRegion.Enabled = SelectionNotEmpty
        ButtonRegionUp.Enabled = SelectionNotEmpty
        ButtonRegionDown.Enabled = SelectionNotEmpty

    End Sub

    Private Sub ButtonNewRegion_Click(sender As Object, e As EventArgs) Handles ButtonNewRegion.Click

        Dim EditForm As New FormEditRegion
        EditForm.TB_Name.Text = "Name"
        If EditForm.ShowDialog(Me) <> Windows.Forms.DialogResult.OK Then Return

        Dim CreatedRegion = New Region
        CreatedRegion.Name = MakeIdentifier(EditForm.TB_Name.Text)

        Dim InsertionPosition = Math.Max(0, LB_Regions.SelectedIndex + 1)
        DefinedRegions.Insert(InsertionPosition, CreatedRegion)

        ' ensure we auto-select the new region
        LB_Regions.SelectedIndex = InsertionPosition
        ForceListUpdates()

        ActiveRegion = LB_Regions.SelectedValue
        ActiveMatrix = Nothing
        ReadRegionProperties()
        ReadMatrixProperties()

    End Sub

    Private Sub ButtonEditRegion_Click(sender As Object, e As EventArgs) Handles ButtonEditRegion.Click

        If ActiveRegion Is Nothing Then Return

        Dim EditForm As New FormEditRegion
        EditForm.TB_Name.Text = ActiveRegion.Name
        If EditForm.ShowDialog(Me) <> Windows.Forms.DialogResult.OK Then Return

        ' update name
        ActiveRegion.Name = MakeIdentifier(EditForm.TB_Name.Text)
        ForceListUpdates()

        ' ensure we auto-select the edited region
        ActiveRegion = LB_Regions.SelectedValue
        ActiveMatrix = Nothing
        ReadRegionProperties()
        ReadMatrixProperties()

    End Sub

    Private Sub ButtonRegionUp_Click(sender As Object, e As EventArgs) Handles ButtonRegionUp.Click

        If LB_Regions.SelectedIndex <= 0 Then Return

        Dim OldSelectedIndex = LB_Regions.SelectedIndex
        Dim MovedRegion = DefinedRegions.Item(OldSelectedIndex)

        DefinedRegions.RemoveAt(OldSelectedIndex)
        DefinedRegions.Insert(OldSelectedIndex - 1, MovedRegion)
        LB_Regions.SelectedIndex = OldSelectedIndex - 1
        ForceListUpdates()

        ' ensure we re-select the moved region
        ActiveRegion = LB_Regions.SelectedValue
        ActiveMatrix = Nothing
        ReadRegionProperties()
        ReadMatrixProperties()

    End Sub

    Private Sub ButtonRegionDown_Click(sender As Object, e As EventArgs) Handles ButtonRegionDown.Click

        If LB_Regions.SelectedIndex >= (LB_Regions.Items.Count - 1) Then Return

        Dim OldSelectedIndex = LB_Regions.SelectedIndex
        Dim MovedRegion = DefinedRegions.Item(OldSelectedIndex)

        DefinedRegions.RemoveAt(OldSelectedIndex)
        DefinedRegions.Insert(OldSelectedIndex + 1, MovedRegion)
        LB_Regions.SelectedIndex = OldSelectedIndex + 1
        ForceListUpdates()

        ' ensure we re-select the moved region
        ActiveRegion = LB_Regions.SelectedValue
        ActiveMatrix = Nothing
        ReadRegionProperties()
        ReadMatrixProperties()

    End Sub

    Private Sub ButtonDeleteRegion_Click(sender As Object, e As EventArgs) Handles ButtonDeleteRegion.Click

        Dim OldSelectedIndex = LB_Regions.SelectedIndex
        DefinedRegions.RemoveAt(OldSelectedIndex)
        LB_Regions.SelectedIndex = -1

    End Sub

    ' ---------------------------------
    '   HANDLING THE LIST OF MATRICES
    ' ---------------------------------

    Private Sub LB_Matrices_SelectedIndexChanged(sender As Object, e As EventArgs) Handles LB_Matrices.SelectedIndexChanged

        ActiveMatrix = LB_Matrices.SelectedValue

        If ActiveMatrix Is Nothing Then
            ActiveRegion = Nothing
        Else
            ActiveRegion = ActiveMatrix.FirstRegion
        End If

        ReadMatrixProperties()
        ReadRegionProperties()


        ' update list handling buttons
        Dim SelectionNotEmpty = (LB_Matrices.SelectedIndex >= 0 AndAlso Not ActiveMatrix Is Nothing)
        ButtonEditMatrix.Enabled = SelectionNotEmpty
        ButtonDeleteMatrix.Enabled = SelectionNotEmpty
        ButtonMatrixUp.Enabled = SelectionNotEmpty
        ButtonMatrixDown.Enabled = SelectionNotEmpty

    End Sub

    Private Sub ButtonNewMatrix_Click(sender As Object, e As EventArgs) Handles ButtonNewMatrix.Click

        Dim EditForm As New FormEditRegion
        EditForm.TB_Name.Text = "Name"
        If EditForm.ShowDialog(Me) <> Windows.Forms.DialogResult.OK Then Return

        Dim CreatedMatrix = New RegionMatrix
        CreatedMatrix.FirstRegion.Name = MakeIdentifier(EditForm.TB_Name.Text)

        Dim InsertionPosition = Math.Max(0, LB_Matrices.SelectedIndex + 1)
        DefinedMatrices.Insert(InsertionPosition, CreatedMatrix)

        ' ensure we auto-select the new matrix
        LB_Matrices.SelectedIndex = InsertionPosition
        ForceListUpdates()

    End Sub

    Private Sub ButtonEditMatrix_Click(sender As Object, e As EventArgs) Handles ButtonEditMatrix.Click

        If ActiveMatrix Is Nothing Then Return

        Dim EditForm As New FormEditRegion
        EditForm.TB_Name.Text = ActiveMatrix.FirstRegion.Name
        If EditForm.ShowDialog(Me) <> Windows.Forms.DialogResult.OK Then Return

        ' update name
        ActiveMatrix.FirstRegion.Name = MakeIdentifier(EditForm.TB_Name.Text)
        ForceListUpdates()

    End Sub

    Private Sub ButtonMatrixUp_Click(sender As Object, e As EventArgs) Handles ButtonMatrixUp.Click

        If LB_Matrices.SelectedIndex <= 0 Then Return

        Dim OldSelectedIndex = LB_Matrices.SelectedIndex
        Dim MovedMatrix = DefinedMatrices.Item(OldSelectedIndex)

        DefinedMatrices.RemoveAt(OldSelectedIndex)
        DefinedMatrices.Insert(OldSelectedIndex - 1, MovedMatrix)
        LB_Matrices.SelectedIndex = OldSelectedIndex - 1
        ForceListUpdates()

    End Sub

    Private Sub ButtonMatrixDown_Click(sender As Object, e As EventArgs) Handles ButtonMatrixDown.Click

        If LB_Matrices.SelectedIndex >= (LB_Matrices.Items.Count - 1) Then Return

        Dim OldSelectedIndex = LB_Matrices.SelectedIndex
        Dim MovedMatrix = DefinedMatrices.Item(OldSelectedIndex)

        DefinedMatrices.RemoveAt(OldSelectedIndex)
        DefinedMatrices.Insert(OldSelectedIndex + 1, MovedMatrix)
        LB_Matrices.SelectedIndex = OldSelectedIndex + 1
        ForceListUpdates()

    End Sub

    Private Sub ButtonDeleteMatrix_Click(sender As Object, e As EventArgs) Handles ButtonDeleteMatrix.Click

        Dim OldSelectedIndex = LB_Matrices.SelectedIndex
        DefinedMatrices.RemoveAt(OldSelectedIndex)
        LB_Matrices.SelectedIndex = -1

    End Sub

    ' ----------------------------
    '   IMPORT/EXPORT FUNCTIONS
    ' ----------------------------

    Private Sub ButtonLoadTexture_Click(sender As Object, e As EventArgs) Handles ButtonLoadTexture.Click

        Try
            OpenDialog.Filter = "PNG Images|*.png"
            If OpenDialog.ShowDialog(Me) <> Windows.Forms.DialogResult.OK Then Return
            LoadTexture(OpenDialog.FileName)

            MsgBox("Texture has been loaded", MsgBoxStyle.Information Or MsgBoxStyle.OkOnly, "Success")

        Catch ex As Exception
            MsgBox("Cannot load the texture: " & ex.Message, MsgBoxStyle.Critical Or MsgBoxStyle.OkOnly, "Error")
        End Try

    End Sub

    Private Sub ButtonLoadProject_Click(sender As Object, e As EventArgs) Handles ButtonLoadProject.Click

        Try
            OpenDialog.Filter = "Project Files|*.xml"
            If OpenDialog.ShowDialog(Me) <> Windows.Forms.DialogResult.OK Then Return
            LoadFromXML(OpenDialog.FileName)

            ' on success reset the environment
            ActiveRegion = Nothing
            ActiveMatrix = Nothing
            LB_Regions.SelectedIndex = -1
            LB_Matrices.SelectedIndex = -1
            TabRegionTypes.SelectedTab = TabRegions

            MsgBox("Project has been loaded", MsgBoxStyle.Information Or MsgBoxStyle.OkOnly, "Success")

        Catch ex As Exception
            MsgBox("Cannot load the project: " & ex.Message, MsgBoxStyle.Critical Or MsgBoxStyle.OkOnly, "Error")
        End Try

    End Sub

    Private Sub ButtonSaveProject_Click(sender As Object, e As EventArgs) Handles ButtonSaveProject.Click

        Try
            SaveDialog.Filter = "Project Files|*.xml"
            If SaveDialog.ShowDialog(Me) <> Windows.Forms.DialogResult.OK Then Return
            SaveToXML(SaveDialog.FileName)

            ' on success just report
            MsgBox("Project has been saved", MsgBoxStyle.Information Or MsgBoxStyle.OkOnly, "Success")

        Catch ex As Exception
            MsgBox("Cannot save the project: " & ex.Message, MsgBoxStyle.Critical Or MsgBoxStyle.OkOnly, "Error")
        End Try

    End Sub

    Private Sub ButtonExportHeader_Click(sender As Object, e As EventArgs) Handles ButtonExportHeader.Click

        Try
            SaveDialog.Filter = "C Headers|*.h"
            If SaveDialog.ShowDialog(Me) <> Windows.Forms.DialogResult.OK Then Return
            ExportCHeader(SaveDialog.FileName)

            ' on success just report
            MsgBox("C header has been created", MsgBoxStyle.Information Or MsgBoxStyle.OkOnly, "Success")

        Catch ex As Exception
            MsgBox("Cannot export to C header: " & ex.Message, MsgBoxStyle.Critical Or MsgBoxStyle.OkOnly, "Error")
        End Try

    End Sub

    Private Sub TabRegionTypes_SelectedIndexChanged(sender As Object, e As EventArgs) Handles TabRegionTypes.SelectedIndexChanged

        If MainForm Is Nothing Then Return

        ' ensure that, when changing tab, displayed region changes to match it
        If TabRegionTypes.SelectedTab Is TabRegions Then
            ActiveRegion = LB_Regions.SelectedValue
            ActiveMatrix = Nothing
        End If

        If TabRegionTypes.SelectedTab Is TabMatrices Then
            ActiveMatrix = LB_Matrices.SelectedValue

            If ActiveMatrix Is Nothing Then
                ActiveRegion = Nothing
            Else
                ActiveRegion = ActiveMatrix.FirstRegion
            End If

        End If

        ReadMatrixProperties()
        ReadRegionProperties()

    End Sub

End Class
