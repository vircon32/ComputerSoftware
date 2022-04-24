Imports System.ComponentModel

Public Module RegionData

    ' ------------------------------------------------
    '   GENERAL USE FUNCTIONS
    ' ------------------------------------------------

    Public Function IsValidIdentifierCharacter(c As Char) As Boolean

        If Char.IsLetterOrDigit(c) Then Return True
        If c = "_"c Then Return True
        Return False

    End Function

    Public Function MakeIdentifier(Name As String) As String

        Dim Identifier As String = ""

        ' just keep adding all valid characters
        ' and replacing invalid ones with underscore
        For Each c As Char In Name
            If IsValidIdentifierCharacter(c) Then
                Identifier += c
            Else
                Identifier += "_"c
            End If
        Next

        Return Identifier

    End Function

    ' ------------------------------------------------
    '   DEFINITION OF OUR SPECIFIC DATA STRUCTURES
    ' ------------------------------------------------

    Public Class Region

        ' region identification
        Public Name As String

        ' pixel coordinates of limits (included)
        ' these can never be outside the texture
        Public Top As Integer
        Public Left As Integer
        Public Right As Integer
        Public Bottom As Integer

        ' reference spot, may be anywhere
        ' (outside the region or even the texture)
        Public HotspotX As Integer
        Public HotspotY As Integer

        ' constructor with default values
        Public Sub New()
            Name = "Name"
            Top = 0
            Bottom = 0
            Left = 0
            Right = 0
            HotspotX = 0
            HotspotY = 0
        End Sub

        ' this is needed for listbox binding!
        Public Property GetName As String
            Get
                Return Name
            End Get
            Set(value As String)
                Name = value
            End Set
        End Property

        ' moves the whole region, checking for texture limits
        Public Sub MoveRegion(OffsetX As Integer, OffsetY As Integer, TextureWidth As Integer, TextureHeight As Integer)

            ' limit movement on X
            Dim MinOffsetX = 0 - Left
            Dim MaxOffsetX = (TextureWidth - 1) - Right

            If OffsetX < MinOffsetX Then OffsetX = MinOffsetX
            If OffsetX > MaxOffsetX Then OffsetX = MaxOffsetX

            ' limit movement on Y
            Dim MinOffsetY = 0 - Top
            Dim MaxOffsetY = (TextureHeight - 1) - Bottom

            If OffsetY < MinOffsetY Then OffsetY = MinOffsetY
            If OffsetY > MaxOffsetY Then OffsetY = MaxOffsetY

            ' actually move region
            Left += OffsetX
            Right += OffsetX
            HotspotX += OffsetX

            Top += OffsetY
            Bottom += OffsetY
            HotspotY += OffsetY

        End Sub

    End Class

    Public Class RegionMatrix

        Public FirstRegion As Region
        Public Columns As Integer
        Public Rows As Integer
        Public Separation As Integer

        ' constructor with default values
        Public Sub New()
            FirstRegion = New Region
            FirstRegion.Name = "FirstRegion"
            Columns = 1
            Rows = 1
            Separation = 0
        End Sub

        ' this is needed for listbox binding!
        Public Property GetName As String
            Get
                Return FirstRegion.Name
            End Get
            Set(value As String)
                FirstRegion.Name = value
            End Set
        End Property

    End Class

    ' ----------------------------------------------
    '   GLOBAL VARIABLES TO STORE DEFINITIONS
    ' ----------------------------------------------

    Public LoadedTexture As Image = Nothing
    Public TextureName As String
    Public TexturePath As String

    Public ActiveRegion As Region = Nothing
    Public ActiveMatrix As RegionMatrix = Nothing

    Public DefinedRegions As New BindingList(Of Region)
    Public DefinedMatrices As New BindingList(Of RegionMatrix)

    ' we need the instance of the main form
    Public MainForm As FormEditor = Nothing

    ' ----------------------------------------------
    '   READ FUNCTIONS
    ' ----------------------------------------------

    Public Sub ReadRegionProperties()

        MainForm.PanelLimits.Enabled = (Not ActiveRegion Is Nothing)
        MainForm.PanelHotspot.Enabled = (Not ActiveRegion Is Nothing)

        ' if there is no active region, set defaults
        If ActiveRegion Is Nothing Then

            MainForm.TB_Left.Text = 0
            MainForm.TB_Right.Text = 0
            MainForm.TB_Top.Text = 0
            MainForm.TB_Bottom.Text = 0
            MainForm.TB_HotspotX.Text = 0
            MainForm.TB_HotspotY.Text = 0
            Return

        End If

        MainForm.TB_Left.Text = ActiveRegion.Left
        MainForm.TB_Right.Text = ActiveRegion.Right
        MainForm.TB_Top.Text = ActiveRegion.Top
        MainForm.TB_Bottom.Text = ActiveRegion.Bottom
        MainForm.TB_HotspotX.Text = ActiveRegion.HotspotX
        MainForm.TB_HotspotY.Text = ActiveRegion.HotspotY

    End Sub

    Public Sub ReadMatrixProperties()

        MainForm.PanelMatrix.Enabled = (Not ActiveMatrix Is Nothing)

        ' if there is no active region, set default values
        If ActiveMatrix Is Nothing Then

            MainForm.TB_Columns.Text = 1
            MainForm.TB_Rows.Text = 1
            MainForm.TB_Separation.Text = 0
            Return

        End If

        MainForm.TB_Columns.Text = ActiveMatrix.Columns
        MainForm.TB_Rows.Text = ActiveMatrix.Rows
        MainForm.TB_Separation.Text = ActiveMatrix.Separation

    End Sub

    ' ----------------------------------------------
    '   WRITE FUNCTIONS
    ' ----------------------------------------------

    Public Sub WriteXCoordinate(ByRef TargetVariable As Integer, NumberText As String, ClampToTexture As Boolean)

        Dim NewCoordinate As Integer = 0
        If Not Integer.TryParse(NumberText, NewCoordinate) Then Return

        If ClampToTexture Then
            If NewCoordinate < 0 Then NewCoordinate = 0
            If NewCoordinate >= LoadedTexture.Width Then NewCoordinate = LoadedTexture.Width - 1
        End If

        TargetVariable = NewCoordinate
        ReadRegionProperties()
        ReadMatrixProperties()

    End Sub

    Public Sub WriteYCoordinate(ByRef TargetVariable As Integer, NumberText As String, ClampToTexture As Boolean)

        Dim NewCoordinate As Integer = 0
        If Not Integer.TryParse(NumberText, NewCoordinate) Then Return

        If ClampToTexture Then
            If NewCoordinate < 0 Then NewCoordinate = 0
            If NewCoordinate >= LoadedTexture.Height Then NewCoordinate = LoadedTexture.Height - 1
        End If

        TargetVariable = NewCoordinate
        ReadRegionProperties()
        ReadMatrixProperties()

    End Sub

    Public Sub WriteMatrixDimension(ByRef TargetVariable As Integer, NumberText As String)

        Dim NewDimension As Integer = 0
        If Not Integer.TryParse(NumberText, NewDimension) Then Return

        ' clamp to sane values
        If NewDimension < 1 Then NewDimension = 1
        If NewDimension > 1024 Then NewDimension = 1024

        TargetVariable = NewDimension
        ReadRegionProperties()
        ReadMatrixProperties()

    End Sub

End Module
