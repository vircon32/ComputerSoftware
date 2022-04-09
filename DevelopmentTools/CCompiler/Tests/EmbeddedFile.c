embedded int[2][3] matrix = "EmbeddedMatrix.bin";

void main()
{
    matrix[1][2] = 25;
    matrix[0][0] = matrix[1][2];
}
