/*===================================================

	DEFINITIONS

===================================================*/

//Multiple definition prevention
#ifndef BBInterface_Definitions_h
#define BBInterface_Definitions_h

//Plugin information
extern const wchar_t szAppName       [];
extern const wchar_t szVersion       [];
extern const wchar_t szInfoVersion   [];
extern const wchar_t szInfoAuthor    [];
extern const wchar_t szInfoRelDate   [];
extern const wchar_t szInfoLink      [];
extern const wchar_t szInfoEmail     [];

//Local variables
extern const wchar_t szPluginAbout   [];
extern const wchar_t szPluginAboutLastControl    [];
extern const wchar_t szPluginAboutQuickRef   [];

//Strings used frequently
extern const int szBBroamLength;
extern const wchar_t szBBroam            [];
extern const wchar_t szBEntityControl    [];
extern const wchar_t szBEntityAgent      [];
extern const wchar_t szBEntityPlugin     [];
extern const wchar_t szBEntityVarset     [];
extern const wchar_t szBEntityWindow     [];
extern const wchar_t szBEntityModule     [];
extern const wchar_t szBActionCreate     [];
extern const wchar_t szBActionCreateChild[];
extern const wchar_t szBActionDelete     [];
extern const wchar_t szBActionSetAgent   [];
extern const wchar_t szBActionRemoveAgent[];
extern const wchar_t szBActionSetAgentProperty   [];
extern const wchar_t szBActionSetControlProperty [];
extern const wchar_t szBActionSetWindowProperty  [];
extern const wchar_t szBActionSetPluginProperty  [];
extern const wchar_t szBActionSetModuleProperty  [];


extern const wchar_t szBActionSetDefault			[];
extern const wchar_t szBActionAssignToModule		[];
extern const wchar_t szBActionDetachFromModule		[];
extern const wchar_t szBActionOnLoad				[];
extern const wchar_t szBActionOnUnload				[];

extern const wchar_t szBActionRename     [];
extern const wchar_t szBActionLoad       [];
extern const wchar_t szBActionEdit       [];
extern const wchar_t szBActionToggle     [];
extern const wchar_t szBActionSave       [];
extern const wchar_t szBActionSaveAs     [];
extern const wchar_t szBActionRevert     [];
extern const wchar_t szBActionAbout      [];

extern const wchar_t szTrue  [];
extern const wchar_t szFalse [];

extern const wchar_t szFilterProgram [];
extern const wchar_t szFilterScript  [];
extern const wchar_t szFilterAll [];

//Convenient arrays of strings
extern const wchar_t *szBoolArray[2];


//Constant data referrals
#define DATAFETCH_INT_DEFAULTHEIGHT 1101
#define DATAFETCH_INT_DEFAULTWIDTH 1102
#define DATAFETCH_INT_MIN_WIDTH 1201
#define DATAFETCH_INT_MIN_HEIGHT 1202
#define DATAFETCH_INT_MAX_WIDTH 1203
#define DATAFETCH_INT_MAX_HEIGHT 1204
#define DATAFETCH_VALUE_SCALE 2001
#define DATAFETCH_VALUE_BOOL 2002
#define DATAFETCH_VALUE_TEXT 2005

#define DATAFETCH_SUBAGENTS_COUNT 10000
#define DATAFETCH_SUBAGENTS_NAMES_ARRAY 10001
#define DATAFETCH_SUBAGENTS_POINTERS_ARRAY 10002
#define DATAFETCH_SUBAGENTS_TYPES_ARRAY 10003

#define DATAFETCH_CONTENTSIZES 1210 

//Constant control formats
enum CONTROL_FORMAT {
	CONTROL_FORMAT_NONE = 0,
	CONTROL_FORMAT_TRIGGER = (1 << 0),
	CONTROL_FORMAT_DROP = (1 << 1),
	CONTROL_FORMAT_BOOL = (1 << 2),
	CONTROL_FORMAT_INTEGER = (1 << 3),
	CONTROL_FORMAT_DOUBLE = (1 << 4),
	CONTROL_FORMAT_SCALE = (1 << 5),
	CONTROL_FORMAT_TEXT = (1 << 6),
	CONTROL_FORMAT_IMAGE = (1 << 7),
	CONTROL_FORMAT_PLUGIN = (1 << 8)
};

//Constant notify values
enum NOTIFY_TYPE {
	NOTIFY_NOTHING = 0,
	NOTIFY_CHANGE = 100,
	NOTIFY_RESIZE = 200,
	NOTIFY_MOVE = 201,
	NOTIFY_NEEDUPDATE = 300,
	NOTIFY_SAVE_CONTROL = 400,
	NOTIFY_SAVE_AGENT = 401,
	NOTIFY_SAVE_CONTROLTYPE = 402,
	NOTIFY_SAVE_AGENTTYPE = 403,
	NOTIFY_DRAW = 600,
	NOTIFY_DRAGACCEPT = 601,
	NOTIFY_TIMER = 700
};

/*=================================================*/
#endif // BBInterface_Definitions_h

