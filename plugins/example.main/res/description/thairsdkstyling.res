CONTAINER Thairsdkstyling
{
	NAME Thairsdkstyling;
	INCLUDE Texpression;

	GROUP ID_TAGPROPERTIES
	{
		REAL HAIR_STYLING_DISPLACE { UNIT METER; MIN 0.0; }

		SPLINE HAIR_STYLING_SPLINE_X
		{ 
			SHOWGRID_H; 
			SHOWGRID_V; 

			MINSIZE_H 120;
			MINSIZE_V 90; 

			EDIT_H; 
			EDIT_V; 

			X_MIN 0; 
			X_MAX 1; 

			Y_MIN 0; 
			Y_MAX 1; 

			X_STEPS 1; 
			Y_STEPS 1;
		}	

		SPLINE HAIR_STYLING_SPLINE_Y
		{ 
			SHOWGRID_H; 
			SHOWGRID_V; 

			MINSIZE_H 120;
			MINSIZE_V 90; 

			EDIT_H; 
			EDIT_V; 

			X_MIN 0; 
			X_MAX 1; 

			Y_MIN 0; 
			Y_MAX 1; 

			X_STEPS 1; 
			Y_STEPS 1;
		}
	}
}
