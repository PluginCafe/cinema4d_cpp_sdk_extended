CONTAINER oshufflingparticles
{
	NAME oshufflingparticles;
	INCLUDE Obase;

	GROUP ID_OBJECTPROPERTIES
	{
		GROUP
		{
			LAYOUTGROUP; COLUMNS 3;

			GROUP
			{
				REAL SDK_EXAMPLE_SHUFFLINGPARTICLES_SIZE_X   {UNIT METER; MIN 1.0;}
			}
			GROUP
			{
				REAL SDK_EXAMPLE_SHUFFLINGPARTICLES_SIZE_Y   {UNIT METER; MIN 1.0;}
			}
			GROUP
			{
				REAL SDK_EXAMPLE_SHUFFLINGPARTICLES_SIZE_Z   {UNIT METER; MIN 1.0;}
			}
		}
		LONG SDK_EXAMPLE_SHUFFLINGPARTICLES_SEED 		{ MIN 0; }
		LONG SDK_EXAMPLE_SHUFFLINGPARTICLES_STRENGTH { MIN 1; MAX 100;}
	}
}
