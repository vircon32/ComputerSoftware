Public Class CustomPictureBox
    Inherits PictureBox

    Private Zoom As Integer = 1
    Public DashOffset As Double = 0
    Public ShowHotspot As Boolean = True

    Protected Overrides Sub OnPaintBackground(pevent As PaintEventArgs)

        pevent.Graphics.InterpolationMode = Drawing2D.InterpolationMode.NearestNeighbor
        MyBase.OnPaintBackground(pevent)

        ' nothing more to do it there is no image or active region
        If BackgroundImage Is Nothing Then Return
        If ActiveRegion Is Nothing Then Return

        ' create pens with some degree of opacity
        Dim RegionBrush = New SolidBrush(Color.FromArgb(80, MainForm.ButtonColorRegion.BackColor))
        Dim RegionPen = New Pen(Color.FromArgb(160, MainForm.ButtonColorRegion.BackColor), Zoom)
        Dim HotspotPen = New Pen(Color.FromArgb(160, MainForm.ButtonColorHotspot.BackColor), Zoom)
        RegionPen.DashStyle = Drawing2D.DashStyle.Dash
        RegionPen.DashCap = Drawing2D.DashCap.Flat
        RegionPen.DashOffset = DashOffset

        ' now draw our rectangle
        Dim ZoomedLeft As Integer = Zoom * ActiveRegion.Left
        Dim ZoomedTop As Integer = Zoom * ActiveRegion.Top
        Dim ZoomedWidth As Integer = Zoom * (ActiveRegion.Right - ActiveRegion.Left)
        Dim ZoomedHeight As Integer = Zoom * (ActiveRegion.Bottom - ActiveRegion.Top)

        pevent.Graphics.DrawRectangle(RegionPen, New Rectangle(ZoomedLeft, ZoomedTop, Math.Max(1, ZoomedWidth), Math.Max(1, ZoomedHeight)))
        pevent.Graphics.FillRectangle(RegionBrush, New Rectangle(ZoomedLeft, ZoomedTop, Math.Max(1, ZoomedWidth), Math.Max(1, ZoomedHeight)))

        ' draw the image's hotspot
        If ShowHotspot Then
            Dim ZoomedHotspotX As Integer = Zoom * ActiveRegion.HotspotX
            Dim ZoomedHotspotY As Integer = Zoom * ActiveRegion.HotspotY
            pevent.Graphics.DrawLine(HotspotPen, New Point(ZoomedHotspotX - 2 * Zoom, ZoomedHotspotY), New Point(ZoomedHotspotX + 2 * Zoom, ZoomedHotspotY))
            pevent.Graphics.DrawLine(HotspotPen, New Point(ZoomedHotspotX, ZoomedHotspotY - 2 * Zoom), New Point(ZoomedHotspotX, ZoomedHotspotY + 2 * Zoom))
        End If

        ' draw the matrix when applicable
        If ActiveMatrix Is Nothing Then Return

        For Row As Integer = 1 To ActiveMatrix.Rows
            For Column As Integer = 1 To ActiveMatrix.Columns

                ' first region is already drawn
                If Row = 1 AndAlso Column = 1 Then Continue For

                Dim CurrentTop = Zoom * ActiveRegion.Top + Zoom * (Row - 1) * (ActiveRegion.Bottom - ActiveRegion.Top + 1 + ActiveMatrix.Separation)
                Dim CurrentLeft = ZoomedLeft + Zoom * (Column - 1) * (ActiveRegion.Right - ActiveRegion.Left + 1 + ActiveMatrix.Separation)
                pevent.Graphics.DrawRectangle(RegionPen, New Rectangle(CurrentLeft, CurrentTop, Math.Max(1, ZoomedWidth), Math.Max(1, ZoomedHeight)))

            Next
        Next

    End Sub

    Public Property ZoomLevel As Integer
        Get
            Return Zoom
        End Get
        Set(value As Integer)

            If BackgroundImage Is Nothing Then Return

            Zoom = value
            If Zoom < 1 Then Zoom = 1
            If Zoom > 8 Then Zoom = 8

            Width = BackgroundImage.Width * Zoom
            Height = BackgroundImage.Height * Zoom

        End Set
    End Property

End Class
