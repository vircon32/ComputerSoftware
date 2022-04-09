int GetCycleCounter()
{
    int CycleCounter;
    
    asm
    {
        "push R0"
        "in R0, TIM_CycleCounter"
        "mov {CycleCounter}, R0"
        "pop R0"
    }
    
    return CycleCounter;
}


void main( void )
{
    int Cycles = GetCycleCounter();
    Cycles += 1;
}