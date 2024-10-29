Imports System.Xml

Public Module FileExports


    Public Sub LoadTexture(FilePath As String)

        LoadedTexture = Image.FromFile(FilePath)
        MainForm.PictureTexture.BackgroundImage = LoadedTexture
        MainForm.PictureTexture.Size = LoadedTexture.Size

        ' set texture name from the file name
        Dim FileName = IO.Path.GetFileNameWithoutExtension(FilePath)
        MainForm.TB_TextureName.Text = MakeIdentifier(FileName)
        TextureName = MainForm.TB_TextureName.Text

        ' save the loaded texture path
        TexturePath = FilePath

        ' from this moment view and tools are usable
        MainForm.PanelTools.Enabled = True
        MainForm.PanelTexture.Enabled = True

        ' also from now the project is not empty so
        ' many actions begin to make sense
        MainForm.PanelRegionLists.Enabled = True
        MainForm.ButtonSaveProject.Enabled = True
        MainForm.ButtonExportHeader.Enabled = True
        MainForm.ButtonExportASM.Enabled = True

    End Sub

    ' ----------------------------------------------
    '   XML LOAD FUNCTIONS
    ' ----------------------------------------------

    Public Sub LoadRegionFromXML(NewRegion As Region, RegionElement As XmlElement)

        NewRegion.Name = RegionElement.GetAttribute("name")
        NewRegion.Left = Integer.Parse(RegionElement.GetAttribute("left"))
        NewRegion.Right = Integer.Parse(RegionElement.GetAttribute("right"))
        NewRegion.Top = Integer.Parse(RegionElement.GetAttribute("top"))
        NewRegion.Bottom = Integer.Parse(RegionElement.GetAttribute("bottom"))
        NewRegion.HotspotX = Integer.Parse(RegionElement.GetAttribute("hotspot-x"))
        NewRegion.HotspotY = Integer.Parse(RegionElement.GetAttribute("hotspot-y"))

    End Sub

    Public Sub LoadMatrixFromXML(NewMatrix As RegionMatrix, MatrixElement As XmlElement)

        NewMatrix.Columns = Integer.Parse(MatrixElement.GetAttribute("columns"))
        NewMatrix.Rows = Integer.Parse(MatrixElement.GetAttribute("rows"))
        NewMatrix.Separation = Integer.Parse(MatrixElement.GetAttribute("separation"))

        Dim FirstRegionElement = MatrixElement("region")
        LoadRegionFromXML(NewMatrix.FirstRegion, FirstRegionElement)

    End Sub

    Public Sub LoadFromXML(FilePath As String)

        Dim InputStream As New IO.FileStream(FilePath, IO.FileMode.Open, IO.FileAccess.Read)

        Dim XMLDoc As New XmlDataDocument
        XMLDoc.Load(InputStream)

        Dim Root As XmlElement = XMLDoc.DocumentElement

        If Root.Name <> "texture-regions" Then
            Throw New Exception("Invalid XML document")
        End If

        ' load the texture
        Dim TextureElement = Root("texture")
        TextureName = TextureElement.GetAttribute("name")
        Dim RelativeTexturePath = TextureElement.GetAttribute("path")

        ' texture path is given as relative to the project folder,
        ' so we need to convert it back to absolute or load will fail
        Dim XmlURI As New Uri(FilePath)
        Dim TextureURI = New Uri(XmlURI, RelativeTexturePath)
        TexturePath = TextureURI.AbsolutePath
        LoadTexture(TexturePath)

        ' clear previous definitions
        DefinedRegions.Clear()
        DefinedMatrices.Clear()

        ' and load the new ones
        Dim RegionList = Root.SelectNodes("region")   ' do this, or we will also get the internal first regions in matrices!
        Dim MatrixList = Root.GetElementsByTagName("matrix")

        For Each RegionElement As XmlElement In RegionList

            Dim NewRegion As New Region
            LoadRegionFromXML(NewRegion, RegionElement)
            DefinedRegions.Add(NewRegion)

        Next

        For Each MatrixElement As XmlElement In MatrixList

            Dim NewMatrix As New RegionMatrix
            LoadMatrixFromXML(NewMatrix, MatrixElement)
            DefinedMatrices.Add(NewMatrix)

        Next

        InputStream.Close()

    End Sub

    ' ----------------------------------------------
    '   XML SAVE FUNCTIONS
    ' ----------------------------------------------

    Public Sub SaveRegionToXML(R As Region, HeaderFile As IO.StreamWriter)

        HeaderFile.Write("    <region")
        HeaderFile.Write(" name=""" & R.Name & """")
        HeaderFile.Write(" left=""" & R.Left & """")
        HeaderFile.Write(" top=""" & R.Top & """")
        HeaderFile.Write(" right=""" & R.Right & """")
        HeaderFile.Write(" bottom=""" & R.Bottom & """")
        HeaderFile.Write(" hotspot-x=""" & R.HotspotX & """")
        HeaderFile.Write(" hotspot-y=""" & R.HotspotY & """")

        HeaderFile.WriteLine("/>")

    End Sub

    Public Sub SaveMatrixToXML(M As RegionMatrix, HeaderFile As IO.StreamWriter)

        HeaderFile.Write("    <matrix")
        HeaderFile.Write(" columns = """ & M.Columns & """")
        HeaderFile.Write(" rows=""" & M.Rows & """")
        HeaderFile.Write(" separation=""" & M.Separation & """")
        HeaderFile.WriteLine(">")

        HeaderFile.Write("    ")
        SaveRegionToXML(M.FirstRegion, HeaderFile)

        HeaderFile.WriteLine("    </matrix>")

    End Sub

    Public Sub SaveToXML(FilePath As String)

        ' we will save texture paths as relative to the project XML
        ' to allow moving or copying folders
        Dim XmlURI As New Uri(FilePath)
        Dim TextureURI As New Uri(TexturePath)
        Dim TextureRelativePath As String = XmlURI.MakeRelativeUri(TextureURI).OriginalString

        ' save the texture
        Dim ProjectFile As New IO.StreamWriter(FilePath, False)
        ProjectFile.WriteLine("<texture-regions>")
        ProjectFile.WriteLine("    <texture name=""" & TextureName & """ path=""" & TextureRelativePath & """/>")

        ' save all single regions
        For Each R As Region In DefinedRegions
            SaveRegionToXML(R, ProjectFile)
        Next

        ' save all region matrices
        For Each M As RegionMatrix In DefinedMatrices
            SaveMatrixToXML(M, ProjectFile)
        Next

        ProjectFile.WriteLine("</texture-regions>")
        ProjectFile.Close()

    End Sub

    ' ----------------------------------------------
    '   C HEADER EXPORT FUNCTIONS
    ' ----------------------------------------------

    Public Sub ExportRegionCodeC(R As Region, HeaderFile As IO.StreamWriter)
        HeaderFile.WriteLine("    select_region( Region" & R.Name & " );")
        HeaderFile.WriteLine("    define_region( " & R.Left & "," & R.Top & ",  " &
                             R.Right & "," & R.Bottom & ",  " &
                             R.HotspotX & "," & R.HotspotY & " );")
    End Sub

    Public Sub ExportMatrixCodeC(M As RegionMatrix, HeaderFile As IO.StreamWriter)
        HeaderFile.WriteLine("    define_region_matrix( FirstRegion" & M.FirstRegion.Name & ", " &
                             M.FirstRegion.Left & "," & M.FirstRegion.Top & ",  " &
                             M.FirstRegion.Right & "," & M.FirstRegion.Bottom & ",  " &
                             M.FirstRegion.HotspotX & "," & M.FirstRegion.HotspotY & ",  " &
                             M.Columns & "," & M.Rows & ",  " &
                             M.Separation & " );")
    End Sub

    Public Sub ExportCHeader(FilePath As String)

        ' write header beginning
        Dim HeaderFile As New IO.StreamWriter(FilePath, False)
        HeaderFile.WriteLine("// begin include guard")
        HeaderFile.WriteLine("#ifndef TEXTUREREGIONS_" & TextureName.ToUpper)
        HeaderFile.WriteLine("#define TEXTUREREGIONS_" & TextureName.ToUpper)

        ' we will assign region IDs automatically
        Dim CurrentID As Integer = 0

        ' define the names of all regions
        If Not DefinedRegions.Count = 0 Then

            HeaderFile.WriteLine()
            HeaderFile.WriteLine("// define names for single regions")

            For Each R As Region In DefinedRegions
                HeaderFile.WriteLine("#define Region" & R.Name & "  " & CurrentID)
                CurrentID += 1
            Next

        End If

        ' define the names of all matrices
        If Not DefinedMatrices.Count = 0 Then

            HeaderFile.WriteLine()
            HeaderFile.WriteLine("// define names for first region in region matrices")

            For Each M As RegionMatrix In DefinedMatrices
                HeaderFile.WriteLine("#define FirstRegion" & M.FirstRegion.Name & "  " & CurrentID)
                CurrentID += M.Columns * M.Rows
            Next

        End If

        ' declare a function to define regions
        HeaderFile.WriteLine()
        HeaderFile.WriteLine("// your initialization code must call this function")
        HeaderFile.WriteLine("void DefineRegions_" & TextureName & "() ")
        HeaderFile.WriteLine("{")
        HeaderFile.WriteLine("    select_texture( " & TextureName & " );")

        ' export all single regions
        If Not DefinedRegions.Count = 0 Then

            HeaderFile.WriteLine()
            HeaderFile.WriteLine("    // define single regions")

            For Each R As Region In DefinedRegions
                ExportRegionCodeC(R, HeaderFile)
            Next

        End If

        ' export all region matrices
        If Not DefinedMatrices.Count = 0 Then

            HeaderFile.WriteLine()
            HeaderFile.WriteLine("    // define region matrices")

            For Each M As RegionMatrix In DefinedMatrices
                ExportMatrixCodeC(M, HeaderFile)
            Next

        End If

        ' write header end
        HeaderFile.WriteLine("}")
        HeaderFile.WriteLine()
        HeaderFile.WriteLine("// end include guard")
        HeaderFile.WriteLine("#endif")
        HeaderFile.Close()

    End Sub

    ' ----------------------------------------------
    '   ASM HEADER EXPORT FUNCTIONS
    ' ----------------------------------------------

    Public Sub ExportRegionCodeASM(R As Region, HeaderFile As IO.StreamWriter)
        HeaderFile.WriteLine("  out GPU_SelectedRegion, Region" & R.Name)
        HeaderFile.WriteLine("  out GPU_RegionMinX, " & R.Left)
        HeaderFile.WriteLine("  out GPU_RegionMinY, " & R.Top)
        HeaderFile.WriteLine("  out GPU_RegionMaxX, " & R.Right)
        HeaderFile.WriteLine("  out GPU_RegionMaxY, " & R.Bottom)
        HeaderFile.WriteLine("  out GPU_RegionHotspotX, " & R.HotspotX)
        HeaderFile.WriteLine("  out GPU_RegionHotspotY, " & R.HotspotY)
        HeaderFile.WriteLine()
    End Sub

    Public Sub ExportMatrixCodeASM(M As RegionMatrix, HeaderFile As IO.StreamWriter)
        HeaderFile.WriteLine("  out GPU_SelectedRegion, FirstRegion" & M.FirstRegion.Name)
        HeaderFile.WriteLine("  mov R0, " & M.FirstRegion.Left)
        HeaderFile.WriteLine("  push R0")
        HeaderFile.WriteLine("  mov R0, " & M.FirstRegion.Top)
        HeaderFile.WriteLine("  push R0")
        HeaderFile.WriteLine("  mov R0, " & M.FirstRegion.Right)
        HeaderFile.WriteLine("  push R0")
        HeaderFile.WriteLine("  mov R0, " & M.FirstRegion.Bottom)
        HeaderFile.WriteLine("  push R0")
        HeaderFile.WriteLine("  mov R0, " & M.FirstRegion.HotspotX)
        HeaderFile.WriteLine("  push R0")
        HeaderFile.WriteLine("  mov R0, " & M.FirstRegion.HotspotY)
        HeaderFile.WriteLine("  push R0")
        HeaderFile.WriteLine("  mov R0, " & M.Columns)
        HeaderFile.WriteLine("  push R0")
        HeaderFile.WriteLine("  mov R0, " & M.Rows)
        HeaderFile.WriteLine("  push R0")
        HeaderFile.WriteLine("  mov R0, " & M.Separation)
        HeaderFile.WriteLine("  push R0")
        HeaderFile.WriteLine("  call _define_region_matrix")
        HeaderFile.WriteLine()
    End Sub

    Public Sub ExportASMHeader(FilePath As String)

        ' write header beginning
        Dim HeaderFile As New IO.StreamWriter(FilePath, False)
        HeaderFile.WriteLine("; begin include guard")
        HeaderFile.WriteLine("%ifndef TEXTUREREGIONS_" & TextureName.ToUpper)
        HeaderFile.WriteLine("%define TEXTUREREGIONS_" & TextureName.ToUpper)

        ' include matrix definition header only when needed
        If (DefinedMatrices.Count > 0) Then
            HeaderFile.WriteLine()
            HeaderFile.WriteLine("; include header to define region matrices")
            HeaderFile.WriteLine("%include ""DefineRegionMatrix.asm""")
        End If

        ' we will assign region IDs automatically
        Dim CurrentID As Integer = 0

        ' define the names of all regions
        If Not DefinedRegions.Count = 0 Then

            HeaderFile.WriteLine()
            HeaderFile.WriteLine("; define names for single regions")

            For Each R As Region In DefinedRegions
                HeaderFile.WriteLine("%define Region" & R.Name & "  " & CurrentID)
                CurrentID += 1
            Next

        End If

        ' define the names of all matrices
        If Not DefinedMatrices.Count = 0 Then

            HeaderFile.WriteLine()
            HeaderFile.WriteLine("; define names for first region in region matrices")

            For Each M As RegionMatrix In DefinedMatrices
                HeaderFile.WriteLine("%define FirstRegion" & M.FirstRegion.Name & "  " & CurrentID)
                CurrentID += M.Columns * M.Rows
            Next

        End If

        ' declare a function to define regions
        HeaderFile.WriteLine()
        HeaderFile.WriteLine("; your initialization code must call this subroutine")
        HeaderFile.WriteLine("_define_regions_" & TextureName & ":")
        HeaderFile.WriteLine("  ")
        HeaderFile.WriteLine("  out GPU_SelectedTexture, " & TextureName)

        ' export all single regions
        If Not DefinedRegions.Count = 0 Then

            HeaderFile.WriteLine()
            HeaderFile.WriteLine("  ; ----------------------------------")
            HeaderFile.WriteLine("  ; define single regions")
            HeaderFile.WriteLine()

            For Each R As Region In DefinedRegions
                ExportRegionCodeASM(R, HeaderFile)
            Next

        End If

        ' export all region matrices
        If Not DefinedMatrices.Count = 0 Then

            HeaderFile.WriteLine()
            HeaderFile.WriteLine("  ; ----------------------------------")
            HeaderFile.WriteLine("  ; define region matrices")
            HeaderFile.WriteLine()

            For Each M As RegionMatrix In DefinedMatrices
                ExportMatrixCodeASM(M, HeaderFile)
            Next

        End If

        ' finish subroutine
        HeaderFile.WriteLine()
        HeaderFile.WriteLine("  ; finished, go back")
        HeaderFile.WriteLine("  ret")

        ' write header end
        HeaderFile.WriteLine()
        HeaderFile.WriteLine("; end include guard")
        HeaderFile.WriteLine("%endif")
        HeaderFile.Close()

    End Sub

End Module
