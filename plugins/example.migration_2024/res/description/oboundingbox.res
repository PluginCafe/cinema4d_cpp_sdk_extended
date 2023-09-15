// The description of the #Oboundingbox node.
CONTAINER Oboundingbox
{
  NAME Oboundingbox;
  INCLUDE Obase;

  GROUP ID_OBJECTPROPERTIES
  {
    GROUP
    {
      COLUMNS 2;

      // The display mode parameter of the object, the cycle values reflect the Tdisplay tag values
      // of matching name.
      LONG ID_OBOUNDBOX_DISPLAY_MODE
      {
        CYCLE
        {
          ID_OBOUNDBOX_DISPLAY_MODE_GOURAUD;
          ID_OBOUNDBOX_DISPLAY_MODE_GOURAUD_WIRE;
          ID_OBOUNDBOX_DISPLAY_MODE_QUICK;
          ID_OBOUNDBOX_DISPLAY_MODE_QUICK_WIRE;
          ID_OBOUNDBOX_DISPLAY_MODE_FLAT;
          ID_OBOUNDBOX_DISPLAY_MODE_FLAT_WIRE;
          ID_OBOUNDBOX_DISPLAY_MODE_HIDDENLINE;
          ID_OBOUNDBOX_DISPLAY_MODE_NOSHADING;
        }
      }

      // The button to show the get access count for #ID_OBOUNDBOX_DISPLAY_MODE.
      BUTTON ID_OBOUNDBOX_DISPLAY_MODE_GET_COUNT {}
    }
  }
}
