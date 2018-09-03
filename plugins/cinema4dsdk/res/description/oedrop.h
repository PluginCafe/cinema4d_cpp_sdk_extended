#ifndef OEDROP_H__
#define OEDROP_H__

enum
{
	//groups
	DROPEFFECTOR_GROUPEFFECTOR		= 991,

	//------------------------------
	//Effector controls
	//Generally keep strength at ID 1000
	DROPEFFECTOR_STRENGTH				= 1000,

	//-------------------------------
	//Parameter controls
	//Generally keep these from id 1100 onwards
	DROPEFFECTOR_TARGET					=	1100,
	DROPEFFECTOR_MODE						=	1101,
		DROPEFFECTOR_MODE_AXIS				=	0,
		DROPEFFECTOR_MODE_SELFAXIS		=	1,
		DROPEFFECTOR_MODE_PNORMAL			=	2,
		DROPEFFECTOR_MODE_NNORMAL			=	3,
		DROPEFFECTOR_MODE_PX					=	4,
		DROPEFFECTOR_MODE_PY					=	5,
		DROPEFFECTOR_MODE_PZ					=	6,
		DROPEFFECTOR_MODE_NX					=	7,
		DROPEFFECTOR_MODE_NY					=	8,
		DROPEFFECTOR_MODE_NZ					=	9,
	DROPEFFECTOR_DISTANCE				=	1102
};
#endif // OEDROP_H__
