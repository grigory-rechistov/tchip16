/*    
	tchip16, an open-source Chip16 assembler
    Copyright (C) 2011  Tim Kelsall
	[...]
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _ERROR_H
#define _ERROR_H

#define WAIT char c; std::cin >> c;

enum err {
	ERR_IO, ERR_CMD_NONE, ERR_CMD_UNKNOWN, ERR_OP_UNKNOWN, 
	ERR_OP_ARGS, ERR_NUM_NONE, ERR_LABEL_REDEF, ERR_CONST_REDEF,
	ERR_INC_CYCLE
};

class Error
{
public:
	Error(void);
	Error(int code);
	Error(int code, int lineNb);
	~Error(void);
private:
	void print(int code);
};

#endif
