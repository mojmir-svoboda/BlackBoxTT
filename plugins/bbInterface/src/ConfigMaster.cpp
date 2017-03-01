/*===================================================

	CONFIG MASTER CODE

===================================================*/
// Global Include
#include <blackbox/plugin/bb.h>
#include <stdlib.h>

//Parent include
#include "ConfigMaster.h"

//Includes
#include "AgentMaster.h"
#include "ControlMaster.h"
#include "PluginMaster.h"
#include "WindowMaster.h"
#include "MessageMaster.h"
#include "Definitions.h"
#include "ModuleMaster.h"

// For the calculator part
#include <sstream>
#include <cmath>

//Define these variables
wchar_t *config_masterbroam = nullptr;
FILE * config_file_out = NULL;
bool config_first_line = false;

wchar_t *config_path_plugin = NULL;
wchar_t *config_path_mainscript = NULL;

FILETIME config_mainscript_filetime;

//Internal functions
void config_paths_startup();
void config_paths_shutdown();

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//config_startup
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int config_startup()
{
	config_masterbroam = new wchar_t[BBI_MAX_LINE_LENGTH];
	config_paths_startup();
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//config_shutdown
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int config_shutdown()
{
	delete config_masterbroam;
	config_masterbroam = nullptr;
	config_paths_shutdown();
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//config_save
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int config_save(wchar_t const *filename)
{
	config_file_out = config_open(filename, L"wt");
	if (config_file_out)
	{
		//Save plugin specific data
		plugin_save();

		//Save module data
		module_save_list();

		//Save agent type specific data
		agent_save();

		//Save control type data AND control data
		control_save();

		//Close the file
		fclose(config_file_out);

		if (0 == _wcsicmp(config_path_mainscript, filename))
			check_mainscript_filetime();

		//Save the other files, too
		module_save_all();
		return 0;
	}

	//Must have had an error
	if (!plugin_suppresserrors) BBMessageBox(NULL, L"There was an error saving the configuration file.", szAppName, MB_OK|MB_SYSTEMMODAL);
	return 1;
}
// Just a few utility type quick functions.

inline char* trim_front(char* str)
{
	while (*str <= ' ') ++str;
	return str;
}
inline void trim_back(char* str)
{
	char *p = strchr(str, 0);
	while (p > str && (unsigned char)p[-1] <= ' ') --p;
	*p = 0;
}

// Needs buffer of size BBI_MAX_LINE_LENGTH. Could possibly make it variable size
char *config_read_line(char *buf, FILE* in_file)
{

	if (!fgets(buf, BBI_MAX_LINE_LENGTH, in_file)) return NULL;
	trim_back(buf);
	if (*buf == NULL) return buf;
	char *p;
	while ( *(p = buf +(strlen(buf)-1) ) == '\\')
	{
		char line[BBI_MAX_LINE_LENGTH];
		if(fgets(line, sizeof line, in_file))
		{
			char *newline = trim_front(line); trim_back(newline);
			if (strlen(buf) + strlen(newline) < BBI_MAX_LINE_LENGTH) strcpy(p,newline);
			else { return buf; } // line too long, possible error message. for now it's just an error during later parsing
		} else { return buf; } // FIX, possible error message.
	}
	return buf;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int config_load(wchar_t const *filename, module* caller, const char *section)
{
	//Open the file
	FILE *config_file_in = config_open(filename, L"rt");

	char config_line[BBI_MAX_LINE_LENGTH];
	if (config_file_in)
	{
		bool wanted_section = NULL == section;
		while (config_read_line(config_line, config_file_in))
		{
			if (config_line[0] == '[')
			{
				if (wanted_section) break;
				if (0 == _stricmp(config_line, section))
					wanted_section = true;
				continue;
			}

			if (false == wanted_section)
				continue;

			if (config_line[0] == '@')
			{
				//Interpret the message
				wchar_t config_line_w[BBI_MAX_LINE_LENGTH];
				bb::codecvt_utf8_utf16(config_line, strlen(config_line), config_line_w, BBI_MAX_LINE_LENGTH);
				message_interpret(config_line_w, false, caller);
			}
		}
		//Close the file
		fclose(config_file_in);
		return 0;
	}

	//Must have been an error
	if (!plugin_suppresserrors)
	{
		wchar_t config_line_w[BBI_MAX_LINE_LENGTH];
		swprintf(config_line_w, BBI_MAX_LINE_LENGTH, L"%s:\nThere was an error loading the configuration file.", filename);
		BBMessageBox(NULL, config_line_w, szAppName, MB_OK|MB_SYSTEMMODAL);
	}
	return 1;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//config_backup
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int config_backup(wchar_t *filename)
{
	wchar_t bakfile[MAX_PATH];
	swprintf(bakfile, MAX_PATH,L"%s.bak", filename);

	wchar_t temp[MAX_PATH + 500];
	swprintf(temp, MAX_PATH + 500,
		L"%s"
		L"\n"
		L"\nPlease note that the script format in this version is not compatible "
		L"\nto the previous version."
		L"\n"
		L"\nYour original script will be backed up as:"
		L"\n%s"
		, szVersion, bakfile
	);
	BBMessageBox(NULL, temp, szAppName, MB_OK|MB_SYSTEMMODAL);

	if (!FileExists(bakfile) || IDYES == BBMessageBox(NULL,
			L"Backup file already exists. Overwrite?",
			szAppName, MB_YESNO|MB_SYSTEMMODAL
			))
		CopyFile(filename, bakfile, FALSE);

	return 0;
}

wchar_t * config_makepath(wchar_t *buffer, wchar_t const *filename)
{
	if (NULL == wcschr(filename, L':'))
		return wcscat(wcscpy(buffer, config_path_plugin), filename);
	else
		return wcscpy(buffer, filename);
}

FILE *config_open(wchar_t const * filename, const wchar_t *mode)
{
	wchar_t buffer[MAX_PATH];
	config_first_line = true;
	return _wfopen(config_makepath(buffer, filename), mode);
}

int config_delete(wchar_t const *filename)
{
	wchar_t buffer[MAX_PATH];
	return 0 != ::DeleteFile(config_makepath(buffer, filename));
}

int config_write(char *string)
{
	if(!string) return 0;
	while (*string)
	{	if (*string == '$') fputc('$', config_file_out); //Replace $ with $$
		fputc(*(string++),config_file_out);
	}
	fputc('\n', config_file_out);
	return 0;
}

void config_printf (const char *fmt, ...)
{
	if (config_first_line)
		config_first_line = false;
	else
		fputc('\n', config_file_out);

	va_list arg; va_start(arg, fmt);
	vfprintf (config_file_out, fmt, arg);
	fputc('\n', config_file_out);
}

void config_printf_noskip (const char *fmt, ...)
{
	va_list arg; va_start(arg, fmt);
	vfprintf (config_file_out, fmt, arg);
	fputc('\n', config_file_out);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int getftime(const wchar_t *fn, FILETIME *ft)
{
	HANDLE hf;
	hf = CreateFile(fn, 0, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == hf)
		return 0;
	int r = GetFileTime(hf, NULL, NULL, ft);
	CloseHandle(hf);
	return r!=0;
}

int check_filetime(const wchar_t *fn, FILETIME *ft)
{
	FILETIME t0;
	if (getftime (fn, &t0) && CompareFileTime(&t0, ft) > 0)
	{
		*ft = t0;
		return TRUE;
	}
	return FALSE;
}

int check_mainscript_filetime(void)
{
	return check_filetime(config_path_mainscript, &config_mainscript_filetime);
}

bool check_mainscript_version(void)
{
	//Open the file
	FILE *config_file_in = config_open(config_path_mainscript, L"rt");

	char config_line[BBI_MAX_LINE_LENGTH];
	if (config_file_in)
	{	
		if (	fgets(config_line, sizeof config_line, config_file_in)
			&&	config_line[0] == '!'
			&&	strstr(config_line, "0.9.9")
			) { fclose(config_file_in); return true; }

		fclose(config_file_in);
	}
	return false;
}
//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//For expression interpretation.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Couldn't find a round() function in these include files...
int roundtoint(double d)
{
	if (d >= 0.0) return ceil(d - 0.5);
	else return floor(d + 0.5);
}

class Calculator
{
    std::istream& in;
    enum Token_type
    {
        NUMBER,
        END,
		LEQ, // <=
		GEQ, // >=
		NEQ, // !=
        PLUS = '+',
        MINUS = '-',
        MUL = '*',
        DIV = '/',
        LP = '(',
        RP = ')',
		LT = '<',
		GT = '>',
		EQ = '=',
		NOT = '!',
		TER1 = '?',
		TER2 = ':',
		AND = '&',
		OR = '|',
		XOR = '^'

    };
    Token_type cur_tok;
    double cur_value;
    double *def_value;

    double expr(bool get);
    double term(bool get);
    double prim(bool get);
	double value(bool get);
    Token_type get_token();

public:
    Calculator(double *def, std::istream & input) : def_value(def), in(input) {};
    bool Eval();
};

/*
A value is a comparison between expressions.
An expression is a sum of terms.
A term is a product of primaries.
Primaries are numbers or parenthesised expressions.
*/

double Calculator::value(bool get)
{
    double left = expr(get);
    
    switch(cur_tok)
    {
        case LT:
            return left < expr(true);
        case GT:
            return left > expr(true);
        case EQ:
            return left = expr(true);
        case LEQ:
            return left <= expr(true);
        case GEQ:
            return left >= expr(true);
        case NEQ:
            return left != expr(true);
		case TER1:
			{
			double mid = expr(true);
			if (cur_tok != TER2) throw "Ternary operator not closed.";
			double right = expr(true);
			return left ? mid : right;
			}
		case AND:
            {
            int il = roundtoint(left);
            int ir = roundtoint(expr(true));
            return il & ir;
            }
        case OR:
            {
            int il = roundtoint(left);
            int ir = roundtoint(expr(true));
            return il | ir;
            }
        case XOR:
            {
            int il = roundtoint(left);
            int ir = roundtoint(expr(true));
            return il ^ ir;
            }
                
        default:
            return left;
    }
}

double Calculator::expr(bool get)
{
    double left = term(get);
    
    for(;;)
        switch(cur_tok)
        {
            case PLUS:
                left += term(true);
                break;
            case MINUS:
                left -= term(true);
                break;
            default:
                return left;
        }
}

double Calculator::term(bool get)
{
    double left = prim(get);
    
    for(;;)
        switch(cur_tok)
        {
            case MUL:
                left *= prim(true);
                break;
            case DIV:
                if (double d = prim(true))
                {
                    left /= d;
                    break;
                } else
                    throw "Divide by zero.";
            default:
                return left;
        }
}

double Calculator::prim(bool get)
{
    if (get) get_token();
    for(;;)
        switch(cur_tok)
        {
            case NUMBER:
            {
                double v = cur_value;
                get_token();
                return v;
            }
            case MINUS:
                return -prim(true);
            case LP:
            {
                double e = value(true);
                if (cur_tok != RP) throw "Parenthesis not closed.";
                get_token();
                return e;
            }
            default:
                throw "Primary expected.";
        }
}

Calculator::Token_type Calculator::get_token()
{
    char ch = 0;
	char ch2 = 0;
    in >> ch;
    
    switch (ch)
    {
        case 0:
            return cur_tok = END;
        case '+':
        case '-':
        case '*':
        case '/':
        case '(':
        case ')':
		case '=':
		case '?':
		case ':':
        case '&':
        case '|':
        case '^':
            return cur_tok = Token_type(ch);
		case '<':
		case '>':
		case '!':
			in >> ch2;
			if (ch2 == '=')
				return	cur_tok = 
                        (ch == '>') ?	GEQ : (
						(ch == '<') ?	LEQ :
										NEQ	);
			else {
				in.putback(ch2);
				return cur_tok = Token_type(ch);
			}
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
//		case '.':
            in.putback(ch);
            in >> cur_value;
            return cur_tok = NUMBER;
		case '%':
            cur_value = *def_value;
            return cur_tok = NUMBER;
        default:
            throw "Unknown token.";
    }
}

bool Calculator::Eval()
{
    try {
        get_token();
        if (cur_tok != END)
			{ *def_value = value(false); return true; }
		return false;
    }
    catch (const char * /*c*/) {
//		For debug purposes.
//		MessageBox(NULL, c, szAppName, MB_OK|MB_SYSTEMMODAL);
        return false;
	//Add extra error messages here.
    }
}

bool config_set_int_expr(char *str, int* value)
{
	std::istringstream expr(str); //Now, this might be not the fastest decision, but it is easier this way on the tokenizer function. Still, it can be optimized for speed separately.
	double temp = *value; //Argh, this conversion stuff got ugly. references could possibly solve this.
	Calculator calc(&temp,expr);
	bool result = calc.Eval();
	*value = (int) temp;
	return result;
}

bool config_set_double_expr(char *str, double* value)
{
	std::istringstream expr(str);
	Calculator calc(value,expr);
	return calc.Eval();
}

bool config_set_double_expr(char *str, double* value, double min, double max)
{
	std::istringstream expr(str);
	Calculator calc(value,expr);
	bool result = calc.Eval();
	if (result)
	{
		if (*value < min) *value = min;
		if (*value > max) *value = max;
	}
	return result;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//Lots of simple functions that are useful
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool config_set_long(wchar_t *string, long *valptr)
{
	long value = _wtol(string);
	if ((value == 0 && !config_isstringzero(string))) return false;
	*valptr = value;
	return true;
}
bool config_set_int(const wchar_t *string, int *valptr)
{
	int value = _wtoi(string);
	if ((value == 0 && !config_isstringzero(string))) return false;
	*valptr = value;
	return true;
}
bool config_set_str(wchar_t *string, wchar_t **valptr)
{
	free_string(valptr);
	if (*string) *valptr = new_string(string);
	return true;
}
bool config_set_str(wchar_t const *string, bbstring & valptr)
{
	if (*string)
		valptr = string;
	return true;
}
bool config_set_double(wchar_t *string, double *valptr)
{
	double value = _wtof(string);
	if ((value == 0 && !config_isstringzero(string))) return false;
	*valptr = value;
	return true;
}
bool config_set_long(wchar_t *string, long *valptr, long min, long max)
{
	long value = _wtol(string);
	if (value < min || value > max || (value == 0 && !config_isstringzero(string))) return false;
	*valptr = value;
	return true;
}
bool config_set_int(const wchar_t *string, int *valptr, int min, int max)
{
	int value = _wtoi(string);
	if (value < min || value > max || (value == 0 && !config_isstringzero(string))) return false;
	*valptr = value;
	return true;
}
bool config_set_double(wchar_t *string, double *valptr, double min, double max)
{
	double value = _wtof(string);
	if (value < min || value > max || (value == 0 && !config_isstringzero(string))) return false;
	*valptr = value;
	return true;
}
bool config_set_bool(wchar_t *string, bool *valptr)
{
	if (!_wcsicmp(string, szTrue)) {*valptr = true; return true;}
	else if (!_wcsicmp(string, szFalse)) {*valptr = false; return true;}
	return false;
}
bool config_isstringzero(const wchar_t *string)
{
	//The simple case, for speed
	if (string[0] == '0' && string[1] == '\0') return true;
	//The long and grueling case, if it's more complex
	bool isdotused = false;
	bool isvalid = true;
	int index = 0;
	while (isvalid && string[index] != '\0')
	{
		switch(string[index])
		{
			case '0': break;
			case '.': if (isdotused) isvalid= false; isdotused = true; break;
			default: isvalid = false;
		}
		index++;
	}
	return isvalid;
}

wchar_t *config_get_control_create(controltype *ct)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s", szBBroam, szBEntityControl, szBActionCreate, ct->controltypename);   return config_masterbroam;  }
wchar_t *config_get_control_create_named(controltype *ct, control *c)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s", szBBroam, szBEntityControl, szBActionCreate, ct->controltypename, c->controlname);    return config_masterbroam;  }
wchar_t *config_get_control_create_child(control *c_p, controltype *ct)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s", szBBroam, szBEntityControl, szBActionCreateChild, c_p->controlname, ct->controltypename); return config_masterbroam;  }
wchar_t *config_get_control_create_child_named(control *c_p, controltype *ct, control *c)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s %s", szBBroam, szBEntityControl, szBActionCreateChild, c_p->controlname, ct->controltypename, c->controlname);  return config_masterbroam;  }
wchar_t *config_get_control_delete(control *c)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s", szBBroam, szBEntityControl, szBActionDelete, c->controlname);    return config_masterbroam;  }
wchar_t *config_get_control_saveas(control *c, const wchar_t *filename)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s", szBBroam, szBEntityControl, szBActionSaveAs, c->controlname, filename);    return config_masterbroam;  }
wchar_t *config_get_control_renamecontrol_s(control *c)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s", szBBroam, szBEntityControl, szBActionRename, c->controlname);    return config_masterbroam;  }
wchar_t *config_get_control_clone(control *c)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s", szBBroam, szBEntityControl, "Clone", c->controlname);    return config_masterbroam;  }

wchar_t *config_get_control_setagent_s(control *c, const wchar_t *action, const wchar_t *agenttype)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s %s", szBBroam, szBEntityControl, szBActionSetAgent, c->controlname, action, agenttype); return config_masterbroam;  }
wchar_t *config_get_control_setagent_c(control *c, const wchar_t *action, const wchar_t *agenttype, const wchar_t *parameters)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s %s \"%s\"", szBBroam, szBEntityControl, szBActionSetAgent, c->controlname, action, agenttype, parameters);  return config_masterbroam;  }
wchar_t *config_get_control_setagent_b(control *c, const wchar_t *action, const wchar_t *agenttype, const bool *parameters)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s %s %s", szBBroam, szBEntityControl, szBActionSetAgent, c->controlname, action, agenttype, (*parameters ? szTrue : szFalse));    return config_masterbroam;  }
wchar_t *config_get_control_setagent_i(control *c, const wchar_t *action, const wchar_t *agenttype, const int *parameters)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s %s %d", szBBroam, szBEntityControl, szBActionSetAgent, c->controlname, action, agenttype, *parameters); return config_masterbroam;  }
wchar_t *config_get_control_setagent_d(control *c, const wchar_t *action, const wchar_t *agenttype, const double *parameters)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s %s %f", szBBroam, szBEntityControl, szBActionSetAgent, c->controlname, action, agenttype, *parameters); return config_masterbroam;  }

wchar_t *config_get_control_removeagent(control *c, const wchar_t *action)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s", szBBroam, szBEntityControl, szBActionRemoveAgent, c->controlname, action);    return config_masterbroam;  }

wchar_t *config_get_control_setagentprop_s(control *c, const wchar_t *action, const wchar_t *key)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s %s", szBBroam, szBEntityControl, szBActionSetAgentProperty, c->controlname, action, key);  return config_masterbroam;  }
wchar_t *config_get_control_setagentprop_c(control *c, const wchar_t *action, const wchar_t *key, const wchar_t *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s %s \"%s\"", szBBroam, szBEntityControl, szBActionSetAgentProperty, c->controlname, action, key, value); return config_masterbroam;  }
wchar_t *config_get_control_setagentprop_b(control *c, const wchar_t *action, const wchar_t *key, const bool *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s %s %s", szBBroam, szBEntityControl, szBActionSetAgentProperty, c->controlname, action, key, (*value ? szTrue : szFalse));   return config_masterbroam;  }
wchar_t *config_get_control_setagentprop_i(control *c, const wchar_t *action, const wchar_t *key, const int *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s %s %d", szBBroam, szBEntityControl, szBActionSetAgentProperty, c->controlname, action, key, *value);    return config_masterbroam;  }
wchar_t *config_get_control_setagentprop_d(control *c, const wchar_t *action, const wchar_t *key, const double *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s %s %f", szBBroam, szBEntityControl, szBActionSetAgentProperty, c->controlname, action, key, *value);    return config_masterbroam;  }

wchar_t *config_get_control_setcontrolprop_s(control *c, const wchar_t *key)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s", szBBroam, szBEntityControl, szBActionSetControlProperty, c->controlname, key);    return config_masterbroam;  }
wchar_t *config_get_control_setcontrolprop_c(control *c, const wchar_t *key, const wchar_t *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s \"%s\"", szBBroam, szBEntityControl, szBActionSetControlProperty, c->controlname, key, value);  return config_masterbroam;  }
wchar_t *config_get_control_setcontrolprop_b(control *c, const wchar_t *key, const bool *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s %s", szBBroam, szBEntityControl, szBActionSetControlProperty, c->controlname, key, (*value ? szTrue : szFalse));    return config_masterbroam;  }
wchar_t *config_get_control_setcontrolprop_i(control *c, const wchar_t *key, const int *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s %d", szBBroam, szBEntityControl, szBActionSetControlProperty, c->controlname, key, *value); return config_masterbroam;  }
wchar_t *config_get_control_setcontrolprop_d(control *c, const wchar_t *key, const double *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s %f", szBBroam, szBEntityControl, szBActionSetControlProperty, c->controlname, key, *value); return config_masterbroam;  }

wchar_t *config_get_control_setwindowprop_s(control *c, const wchar_t *key)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s", szBBroam, szBEntityControl, szBActionSetWindowProperty, c->controlname, key); return config_masterbroam;  }
wchar_t *config_get_control_setwindowprop_c(control *c, const wchar_t *key, const wchar_t *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s \"%s\"", szBBroam, szBEntityControl, szBActionSetWindowProperty, c->controlname, key, value);   return config_masterbroam;  }
wchar_t *config_get_control_setwindowprop_b(control *c, const wchar_t *key, const bool *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s %s", szBBroam, szBEntityControl, szBActionSetWindowProperty, c->controlname, key, (*value ? szTrue : szFalse)); return config_masterbroam;  }
wchar_t *config_get_control_setwindowprop_i(control *c, const wchar_t *key, const int *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s %d", szBBroam, szBEntityControl, szBActionSetWindowProperty, c->controlname, key, *value);  return config_masterbroam;  }
wchar_t *config_get_control_setwindowprop_d(control *c, const wchar_t *key, const double *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s %f", szBBroam, szBEntityControl, szBActionSetWindowProperty, c->controlname, key, *value);  return config_masterbroam;  }

wchar_t *config_get_agent_setagentprop_s(const wchar_t *agenttype, const wchar_t *key)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s", szBBroam, szBEntityAgent, szBActionSetAgentProperty, agenttype, key); return config_masterbroam;  }
wchar_t *config_get_agent_setagentprop_c(const wchar_t *agenttype, const wchar_t *key, const wchar_t *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s \"%s\"", szBBroam, szBEntityAgent, szBActionSetAgentProperty, agenttype, key, value);   return config_masterbroam;  }
wchar_t *config_get_agent_setagentprop_b(const wchar_t *agenttype, const wchar_t *key, const bool *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s %s", szBBroam, szBEntityAgent, szBActionSetAgentProperty, agenttype, key, (*value ? szTrue : szFalse)); return config_masterbroam;  }
wchar_t *config_get_agent_setagentprop_i(const wchar_t *agenttype, const wchar_t *key, const int *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s %d", szBBroam, szBEntityAgent, szBActionSetAgentProperty, agenttype, key, *value);  return config_masterbroam;  }
wchar_t *config_get_agent_setagentprop_d(const wchar_t *agenttype, const wchar_t *key, const double *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s %f", szBBroam, szBEntityAgent, szBActionSetAgentProperty, agenttype, key, *value);  return config_masterbroam;  }

wchar_t *config_get_control_setpluginprop_s(control *c, const wchar_t *key)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s", szBBroam, szBEntityControl, szBActionSetPluginProperty, c->controlname, key); return config_masterbroam;  }
wchar_t *config_get_control_setpluginprop_b(control *c, const wchar_t *key, const bool *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s %s", szBBroam, szBEntityControl, szBActionSetPluginProperty, c->controlname, key, (*value ? szTrue : szFalse)); return config_masterbroam;  }

wchar_t *config_get_plugin_setpluginprop_s(const wchar_t *key)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s", szBBroam, szBEntityPlugin, szBActionSetPluginProperty, key);   return config_masterbroam;  }
wchar_t *config_get_plugin_setpluginprop_c(const wchar_t *key, const wchar_t *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s \"%s\"", szBBroam, szBEntityPlugin, szBActionSetPluginProperty, key, value ? value : L"");   return config_masterbroam;  }
wchar_t *config_get_plugin_setpluginprop_b(const wchar_t *key, const bool *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s", szBBroam, szBEntityPlugin, szBActionSetPluginProperty, key, (*value ? szTrue : szFalse)); return config_masterbroam;  }
wchar_t *config_get_plugin_setpluginprop_i(const wchar_t *key, const int *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %d", szBBroam, szBEntityPlugin, szBActionSetPluginProperty, key, *value);  return config_masterbroam;  }
wchar_t *config_get_plugin_setpluginprop_d(const wchar_t *key, const double *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %f", szBBroam, szBEntityPlugin, szBActionSetPluginProperty, key, *value);  return config_masterbroam;  }

wchar_t *config_get_plugin_load_dialog()
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s", szBBroam, szBEntityPlugin, szBActionLoad);  return config_masterbroam;  }
wchar_t *config_get_plugin_load(const wchar_t *file)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s \"%s\"", szBBroam, szBEntityPlugin, szBActionLoad, file); return config_masterbroam;  }
wchar_t *config_get_plugin_save()
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s", szBBroam, szBEntityPlugin, szBActionSave);  return config_masterbroam;  }
wchar_t *config_get_plugin_saveas()
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s", szBBroam, szBEntityPlugin, szBActionSaveAs);    return config_masterbroam;  }
wchar_t *config_get_plugin_revert()
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s", szBBroam, szBEntityPlugin, szBActionRevert);    return config_masterbroam;  }
wchar_t *config_get_plugin_edit()
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s", szBBroam, szBEntityPlugin, szBActionEdit);  return config_masterbroam;  }

wchar_t *config_get_module_create()
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s", szBBroam, szBEntityModule, szBActionCreate);  return config_masterbroam;  }
wchar_t *config_get_module_load_dialog()
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s", szBBroam, szBEntityModule, szBActionLoad);  return config_masterbroam;  }
wchar_t *config_get_module_load(module *m)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s \"%s\"", szBBroam, szBEntityModule, szBActionLoad, m->filepath);  return config_masterbroam;  }
wchar_t *config_get_module_toggle(module *m)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s", szBBroam, szBEntityModule, szBActionToggle, m->name);  return config_masterbroam;  }
wchar_t *config_get_module_edit(module *m)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s", szBBroam, szBEntityModule, szBActionEdit, m->name);  return config_masterbroam;  }
wchar_t *config_get_module_setdefault(module *m)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s", szBBroam, szBEntityModule, szBActionSetDefault, m == &globalmodule ? L"*global*" : m->name);  return config_masterbroam;  }

wchar_t *config_get_module_setauthor_s(module *m)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s", szBBroam, szBEntityModule, szBActionSetModuleProperty, m->name, L"Author");  return config_masterbroam;  }
wchar_t *config_get_module_setcomments_s(module *m)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s", szBBroam, szBEntityModule, szBActionSetModuleProperty, m->name, L"Comments");  return config_masterbroam;  }
wchar_t *config_get_module_rename_s(module *m)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s", szBBroam, szBEntityModule, szBActionRename, m->name);    return config_masterbroam;  }
wchar_t *config_get_module_onload_s(module *m)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s", szBBroam, szBEntityModule, szBActionOnLoad, m->name, !m->actions[MODULE_ACTION_ONLOAD].empty() ? m->actions[MODULE_ACTION_ONLOAD] : L"");  return config_masterbroam;  }
wchar_t *config_get_module_onunload_s(module *m)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s", szBBroam, szBEntityModule, szBActionOnUnload, m->name, !m->actions[MODULE_ACTION_ONUNLOAD].empty() ? m->actions[MODULE_ACTION_ONUNLOAD] : L"");  return config_masterbroam;  }


wchar_t *config_get_control_assigntomodule(control *c, module *m)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s %s", szBBroam, szBEntityControl, szBActionAssignToModule, c->controlname, m->name);  return config_masterbroam;  }
wchar_t *config_get_control_detachfrommodule(control *c)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s", szBBroam, szBEntityControl, szBActionDetachFromModule, c->controlname);  return config_masterbroam;  }
wchar_t *config_get_module_onload(module *m)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s \"%s\"", szBBroam, szBEntityModule, szBActionOnLoad, m->name, !m->actions[MODULE_ACTION_ONLOAD].empty() ? m->actions[MODULE_ACTION_ONLOAD] : L"");  return config_masterbroam;  }
wchar_t *config_get_module_onunload(module *m)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s \"%s\"", szBBroam, szBEntityModule, szBActionOnUnload, m->name, !m->actions[MODULE_ACTION_ONUNLOAD].empty() ? m->actions[MODULE_ACTION_ONUNLOAD] : L"");  return config_masterbroam;  }
wchar_t *config_get_plugin_onload()
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s \"%s\"", szBBroam, szBEntityPlugin, szBActionOnLoad, !globalmodule.actions[MODULE_ACTION_ONLOAD].empty() ? globalmodule.actions[MODULE_ACTION_ONLOAD] : L"");  return config_masterbroam;  }
wchar_t *config_get_plugin_onunload()
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s \"%s\"", szBBroam, szBEntityPlugin, szBActionOnUnload, !globalmodule.actions[MODULE_ACTION_ONUNLOAD].empty() ? globalmodule.actions[MODULE_ACTION_ONUNLOAD] : L"");  return config_masterbroam;  }

wchar_t *config_get_variable_set(listnode *ln)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s \"%s\"", szBBroam, szBEntityVarset, ln->key, (wchar_t *)ln->value);  return config_masterbroam;  }
wchar_t *config_get_variable_set_static(listnode *ln)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s \"%s\"", szBBroam, szBEntityVarset, "Static", ln->key, (wchar_t *)ln->value);  return config_masterbroam;  }

//--------------- copied, using fully qualified names

wchar_t *config_getfull_control_create_child(control *c_p, controltype *ct)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s %s", szBBroam, szBEntityControl, szBActionCreateChild, c_p->moduleptr->name, c_p->controlname, ct->controltypename); return config_masterbroam;  }
wchar_t *config_getfull_control_create_child_named(control *c_p, controltype *ct, control *c)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s %s %s", szBBroam, szBEntityControl, szBActionCreateChild, c_p->moduleptr->name, c_p->controlname, ct->controltypename, c->controlname);  return config_masterbroam;  }
wchar_t *config_getfull_control_delete(control *c)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s", szBBroam, szBEntityControl, szBActionDelete, c->moduleptr->name, c->controlname);    return config_masterbroam;  }
wchar_t *config_getfull_control_saveas(control *c, const wchar_t *filename)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s %s", szBBroam, szBEntityControl, szBActionSaveAs, c->moduleptr->name, c->controlname, filename);    return config_masterbroam;  }
wchar_t *config_getfull_control_renamecontrol_s(control *c)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s", szBBroam, szBEntityControl, szBActionRename, c->moduleptr->name, c->controlname);    return config_masterbroam;  }
wchar_t *config_getfull_control_clone(control *c)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s", szBBroam, szBEntityControl, "Clone", c->moduleptr->name, c->controlname);    return config_masterbroam;  }

wchar_t *config_getfull_control_setagent_s(control *c, const wchar_t *action, const wchar_t *agenttype)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s %s %s", szBBroam, szBEntityControl, szBActionSetAgent, c->moduleptr->name, c->controlname, action, agenttype); return config_masterbroam;  }
wchar_t *config_getfull_control_setagent_c(control *c, const wchar_t *action, const wchar_t *agenttype, const wchar_t *parameters)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s %s %s \"%s\"", szBBroam, szBEntityControl, szBActionSetAgent, c->moduleptr->name, c->controlname, action, agenttype, parameters);  return config_masterbroam;  }
wchar_t *config_getfull_control_setagent_b(control *c, const wchar_t *action, const wchar_t *agenttype, const bool *parameters)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s %s %s %s", szBBroam, szBEntityControl, szBActionSetAgent, c->moduleptr->name, c->controlname, action, agenttype, (*parameters ? szTrue : szFalse));    return config_masterbroam;  }
wchar_t *config_getfull_control_setagent_i(control *c, const wchar_t *action, const wchar_t *agenttype, const int *parameters)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s %s %s %d", szBBroam, szBEntityControl, szBActionSetAgent, c->moduleptr->name, c->controlname, action, agenttype, *parameters); return config_masterbroam;  }
wchar_t *config_getfull_control_setagent_d(control *c, const wchar_t *action, const wchar_t *agenttype, const double *parameters)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s %s %s %f", szBBroam, szBEntityControl, szBActionSetAgent, c->moduleptr->name, c->controlname, action, agenttype, *parameters); return config_masterbroam;  }

wchar_t *config_getfull_control_removeagent(control *c, const wchar_t *action)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s %s", szBBroam, szBEntityControl, szBActionRemoveAgent, c->moduleptr->name, c->controlname, action);    return config_masterbroam;  }

wchar_t *config_getfull_control_setagentprop_s(control *c, const wchar_t *action, const wchar_t *key)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s %s %s", szBBroam, szBEntityControl, szBActionSetAgentProperty, c->moduleptr->name, c->controlname, action, key);  return config_masterbroam;  }
wchar_t *config_getfull_control_setagentprop_c(control *c, const wchar_t *action, const wchar_t *key, const wchar_t *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s %s %s \"%s\"", szBBroam, szBEntityControl, szBActionSetAgentProperty, c->moduleptr->name, c->controlname, action, key, value); return config_masterbroam;  }
wchar_t *config_getfull_control_setagentprop_b(control *c, const wchar_t *action, const wchar_t *key, const bool *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s %s %s %s", szBBroam, szBEntityControl, szBActionSetAgentProperty, c->moduleptr->name, c->controlname, action, key, (*value ? szTrue : szFalse));   return config_masterbroam;  }
wchar_t *config_getfull_control_setagentprop_i(control *c, const wchar_t *action, const wchar_t *key, const int *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s %s %s %d", szBBroam, szBEntityControl, szBActionSetAgentProperty, c->moduleptr->name, c->controlname, action, key, *value);    return config_masterbroam;  }
wchar_t *config_getfull_control_setagentprop_d(control *c, const wchar_t *action, const wchar_t *key, const double *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s %s %s %f", szBBroam, szBEntityControl, szBActionSetAgentProperty, c->moduleptr->name, c->controlname, action, key, *value);    return config_masterbroam;  }

wchar_t *config_getfull_control_setcontrolprop_s(control *c, const wchar_t *key)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s %s", szBBroam, szBEntityControl, szBActionSetControlProperty, c->moduleptr->name, c->controlname, key);    return config_masterbroam;  }
wchar_t *config_getfull_control_setcontrolprop_c(control *c, const wchar_t *key, const wchar_t *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s %s \"%s\"", szBBroam, szBEntityControl, szBActionSetControlProperty, c->moduleptr->name, c->controlname, key, value);  return config_masterbroam;  }
wchar_t *config_getfull_control_setcontrolprop_b(control *c, const wchar_t *key, const bool *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s %s %s", szBBroam, szBEntityControl, szBActionSetControlProperty, c->moduleptr->name, c->controlname, key, (*value ? szTrue : szFalse));    return config_masterbroam;  }
wchar_t *config_getfull_control_setcontrolprop_i(control *c, const wchar_t *key, const int *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s %s %d", szBBroam, szBEntityControl, szBActionSetControlProperty, c->moduleptr->name, c->controlname, key, *value); return config_masterbroam;  }
wchar_t *config_getfull_control_setcontrolprop_d(control *c, const wchar_t *key, const double *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s %s %f", szBBroam, szBEntityControl, szBActionSetControlProperty, c->moduleptr->name, c->controlname, key, *value); return config_masterbroam;  }

wchar_t *config_getfull_control_setwindowprop_s(control *c, const wchar_t *key)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s %s", szBBroam, szBEntityControl, szBActionSetWindowProperty, c->moduleptr->name, c->controlname, key); return config_masterbroam;  }
wchar_t *config_getfull_control_setwindowprop_c(control *c, const wchar_t *key, const wchar_t *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s %s \"%s\"", szBBroam, szBEntityControl, szBActionSetWindowProperty, c->moduleptr->name, c->controlname, key, value);   return config_masterbroam;  }
wchar_t *config_getfull_control_setwindowprop_b(control *c, const wchar_t *key, const bool *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s %s %s", szBBroam, szBEntityControl, szBActionSetWindowProperty, c->moduleptr->name, c->controlname, key, (*value ? szTrue : szFalse)); return config_masterbroam;  }
wchar_t *config_getfull_control_setwindowprop_i(control *c, const wchar_t *key, const int *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s %s %d", szBBroam, szBEntityControl, szBActionSetWindowProperty, c->moduleptr->name, c->controlname, key, *value);  return config_masterbroam;  }
wchar_t *config_getfull_control_setwindowprop_d(control *c, const wchar_t *key, const double *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s %s %f", szBBroam, szBEntityControl, szBActionSetWindowProperty, c->moduleptr->name, c->controlname, key, *value);  return config_masterbroam;  }

wchar_t *config_getfull_control_setpluginprop_s(control *c, const wchar_t *key)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s %s", szBBroam, szBEntityControl, szBActionSetPluginProperty, c->moduleptr->name, c->controlname, key); return config_masterbroam;  }
wchar_t *config_getfull_control_setpluginprop_b(control *c, const wchar_t *key, const bool *value)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s %s %s", szBBroam, szBEntityControl, szBActionSetPluginProperty, c->moduleptr->name, c->controlname, key, (*value ? szTrue : szFalse)); return config_masterbroam;  }

wchar_t *config_getfull_control_assigntomodule(control *c, module *m)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s %s", szBBroam, szBEntityControl, szBActionAssignToModule, c->moduleptr->name, c->controlname, m->name);  return config_masterbroam;  }
wchar_t *config_getfull_control_detachfrommodule(control *c)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s", szBBroam, szBEntityControl, szBActionDetachFromModule, c->moduleptr->name, c->controlname);  return config_masterbroam;  }

wchar_t *config_getfull_variable_set_static(module *m, listnode *ln)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s \"%s\"", szBBroam, szBEntityVarset, "Static", m->name, ln->key, (wchar_t *)ln->value);  return config_masterbroam;  }
wchar_t *config_getfull_variable_set_static_s(module *m, listnode *ln)
{   swprintf(config_masterbroam, BBI_MAX_LINE_LENGTH, L"%s %s %s %s:%s", szBBroam, szBEntityVarset, "Static", m->name, ln->key);  return config_masterbroam;  }

//##################################################
//config_paths_startup
//##################################################
void config_paths_startup()
{
	const wchar_t *bbinterfacedotrc = L"BBInterface.rc";
	const wchar_t *bbinterfacerc = L"BBInterfacerc";

	config_path_plugin = new wchar_t[MAX_PATH];
	config_path_mainscript = new wchar_t[MAX_PATH];

	//Create a temporary string
	wchar_t *temp = new wchar_t[MAX_PATH];

	//Get the name of this file
	GetModuleFileName(plugin_instance_plugin, temp, MAX_PATH);

	//Find the last slash
	wchar_t *lastslash = wcsrchr(temp, '\\');
	if (lastslash) lastslash[1] = '\0';

	//Copy the path of the plugin
	wcscpy(config_path_plugin, temp);

	//Find the plugin .rc file
	bool found = false;
	//Try the plugin directory, "BBInterface.rc"
	wcscpy(config_path_mainscript, config_path_plugin);
	wcscat(config_path_mainscript, bbinterfacedotrc);
	if (FileExists(config_path_mainscript)) found = true;
	//If not found, try the plugin directory, "BBInterfacerc"
	if (!found)
	{
		wcscpy(config_path_mainscript, config_path_plugin);
		wcscat(config_path_mainscript, bbinterfacerc);
		if (FileExists(config_path_mainscript)) found = true;
	}
	//If not found, try the Blackbox directory, "BBInterface.rc"
	if (!found)
	{
		GetBlackboxPath(config_path_mainscript, MAX_PATH);
		wcscat(config_path_mainscript, bbinterfacedotrc);
		if (FileExists(config_path_mainscript)) found = true;
	}
	//If not found, try the Blackbox directory, "BBInterfacerc"
	if (!found)
	{
		GetBlackboxPath(config_path_mainscript, MAX_PATH);
		wcscat(config_path_mainscript, bbinterfacerc);
		if (FileExists(config_path_mainscript)) found = true;
	}
	//All hope is lost
	if (!found)
	{
		wcscpy(config_path_mainscript, config_path_plugin);
		wcscat(config_path_mainscript, bbinterfacedotrc);
	}

	//Delete the temporary string
	delete[] temp;
}

//##################################################
//config_paths_shutdown
//##################################################
void config_paths_shutdown()
{
	if (config_path_mainscript) delete config_path_mainscript;
	if (config_path_plugin) delete config_path_plugin;
}
