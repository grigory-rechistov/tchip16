/*    
	tchip16, an open-source Chip16 assembler
    Copyright (C) 2011  Tim Kelsall
	[...]
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <map>
#include <fstream>

// Helper structs
struct label_struct {
	std::string* label;
	int line;
	int address;
	bool hasAddress;

	label_struct* next;
};

struct labels {
	label_struct* head;
	int count;
};

// Assembler class, does the hard work
class Assembler {
public:
	Assembler();
	~Assembler();

	bool openFile(char*);
	bool parseLine();

	void outputFile();

private:
	std::map<char*,int> labels;
	std::map<char*,int> consts;

	std::ifstream input;
	std::ofstream output;
	
	int lineNb;
	int curAddress;
}

#endif