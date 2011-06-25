/*    
	tchip16, an open-source Chip16 assembler
    Copyright (C) 2011  Tim Kelsall
	[...]
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _ERROR_H
#define _ERROR_H

#define WAIT char c; std::cin.get(&c,1)

enum ERROR {
	ERR_NONE, ERR_IO, ERR_CMD_NONE, ERR_NO_INPUT, ERR_CMD_UNKNOWN,  
	ERR_OP_UNKNOWN, ERR_OP_ARGS, ERR_NUM_NONE, ERR_LABEL_REDEF,
	ERR_CONST_REDEF, ERR_INC_CYCLE, ERR_INC_NONE, ERR_TOO_MANY, 
	ERR_NAN, ERR_NUM_OVERFLOW, ERR_STR_NOTENDED
};

class Error
{
public:
	Error(void);
	// only error code
	Error(ERROR);
	// error code, filename, line number, object
	Error(ERROR,std::string&,int,std::string&);
	~Error(void);
private:
	void print(ERROR code);
};

#endif
