CONTAINER Ohyperfile
{
	NAME Ohyperfile;
	INCLUDE Obase;

	GROUP ID_OBJECTPROPERTIES
	{
		GROUP
		{
			LAYOUTGROUP; COLUMNS 2;

			// content of layout group needs to be encapsulated in separate groups
			GROUP
			{
				BUTTON ID_BUTTON_ADD_IMAGE {}
			}
			GROUP
			{
				BUTTON ID_BUTTON_SHOW_IMAGES {}
			}
		}
	}
}
