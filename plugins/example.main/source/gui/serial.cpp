#if 0

// Check for serial number

#include "c4d.h"
#include "c4d_symbols.h"

#define SERIAL_SIZE 12
#define MYPLUGIN_ID	9999999	// CHANGE THIS !!!!

static Bool Validate(Char* nr)
{
	// example check for serial number
	if (nr[0] != 'T')
		return false;
	if (nr[1] != 'E')
		return false;
	if (nr[2] != 'S')
		return false;
	if (nr[3] != 'T')
		return false;
	if (nr[4] != '-')
		return false;
	return nr[5] >= '0' && nr[5] <= '9';
}

class SerialDialog : public GeModalDialog
{
private:
	Char* str;

public:
	explicit SerialDialog(Char* t_str);

	virtual Bool CreateLayout();
	virtual Bool InitValues();
	virtual Bool Command(Int32 id, const BaseContainer& msg);
	virtual Bool AskClose();
};

SerialDialog::SerialDialog(Char* t_str)
{
	str = t_str;
}

Bool SerialDialog::AskClose()
{
	String v;
	GetString(IDC_SERIALNUMBER, v);
	v.GetCString(str, SERIAL_SIZE - 1);
	if (!Validate(str))
	{
		Int32 time = GeGetTimer();
		GeShowMouse(MOUSE_BUSY);
		Enable(IDC_SERIALNUMBER, false);
		while (GeGetTimer() - time < 8000)
		{
		}
		Enable(IDC_SERIALNUMBER, true);
		GeShowMouse(MOUSE_NORMAL);
		return true;
	}
	return false;
}

Bool SerialDialog::CreateLayout()
{
	return GeModalDialog::CreateLayout() && LoadDialogResource(DLG_REGISTER, nullptr, 0);
}

Bool SerialDialog::InitValues()
{
	// first call the parent instance
	if (!GeModalDialog::InitValues())
		return false;
	SetString(IDC_SERIALNUMBER, String(str));
	return true;
}

Bool SerialDialog::Command(Int32 id, const BaseContainer& msg)
{
	// read dialog values when user presses OK
	switch (id)
	{
		case IDC_OK:
			String v;
			GetString(IDC_SERIALNUMBER, v);
			v.GetCString(str, SERIAL_SIZE - 1);
			break;
	}

	return true;
}

static Bool OpenSerialDialog(Char* sn)
{
	SerialDialog dlg(sn);
	return dlg.Open();
}

static Bool CheckSerial()	// call this routine AFTER resources have been initialized
{
	Char data[SERIAL_SIZE], sn[SERIAL_SIZE];

	if (!ReadPluginInfo(MYPLUGIN_ID, data, SERIAL_SIZE))
		goto error;

	if (data[0] != 'O' || data[1] != 'K' || !Validate(data + 4))
		goto error;		// check your data here

	return true;

error:
	// serial number could not be authenticated
	ClearMem(sn, SERIAL_SIZE);

	if (!OpenSerialDialog(sn))
		return false;

	data[0] = 'O';	// write the serial number here
	data[1] = 'K';
	CopyMem(sn, data + 4, SERIAL_SIZE - 4);

	return WritePluginInfo(MYPLUGIN_ID, data, SERIAL_SIZE);
}

#endif
