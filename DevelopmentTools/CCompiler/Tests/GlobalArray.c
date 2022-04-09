// all globals
int*[ 8 ] ptrs;
int[2][3] matrix;
int index = 1;

void main()
{
    ptrs[0];
    ptrs[ 0 ] = NULL;
    ptrs[ 1 ] = ptrs[ 0 ] + 1;
    
    matrix[0][0];
    matrix[0][0] = 12;
    matrix[1][2] = 25;
    
    matrix[1][index] = 11;
    matrix[index][1] = 22;
    matrix[0][0] = matrix[index][1];
}

void error_handler()
{
    
}

//int[2][3] matrix;
//matrix[0] is of type: int[3]
