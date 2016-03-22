#include <ale_interface.hpp>
#include <SDL/SDL_events.h>
#include <SDL/SDL_keysym.h>
#include <SDL/SDL_keysym.h>
#include<cstdio>
#include<sstream>
#include<iomanip>
#include<fstream>
#include<memory>
#include<map>
#include<vector>

#include<sys/time.h>
#include<emucore/M6502/src/System.hxx>

#include "DirectedGraph.hpp"

#ifndef RAM_SIZE
#define RAM_SIZE 0x80
#endif
#define RAM_OFFSET 0x80

// TFG_DIR is defined in the Makefile
#define ROM_FILE TFG_DIR "/montezuma_revenge.bin"
#define FREQUENCY_FILE "state_change_freq.txt"
#define LIVES_ADDRESS 58

using namespace std;

#define AUX_VARIABLES								  \
	System &sys = ale.theOSystem->console().system(); \
	ActionVect legal_actions = ale.getLegalActionSet();

#define TIMED_RESETTING_LOOP(body) do {							   \
		struct timeval start_time, current_time;	 		   \
	gettimeofday(&start_time, NULL);						   \
	while(!gettimeofday(&current_time, NULL) &&				   \
		(current_time.tv_sec - start_time.tv_sec) < SECONDS) { \
		body 												   \
		/* Avoid dying to more effectively explore the game */ \
		if(sys.peek(RAM_OFFSET+LIVES_ADDRESS) == 0)					\
			sys.poke(RAM_OFFSET+LIVES_ADDRESS, 5);							\
	}														   \
} while(0)

unique_ptr<vector<int> > ram_addresses_by_change_frequency(ALEInterface &ale) {
	constexpr int SECONDS = 60*5;
	ifstream frequency_file(FREQUENCY_FILE);
	if(!frequency_file.is_open()) {
		cerr << "Calculating frequency list\n";
		AUX_VARIABLES

		int n_changes[RAM_SIZE] = {0};
		uint8_t previous[RAM_SIZE];
		for(int i=0; i<RAM_SIZE; i++)
			previous[i] = sys.peek(RAM_OFFSET + i);

		TIMED_RESETTING_LOOP({
			for(int i=0; i<RAM_SIZE; i++) {
				uint8_t b = sys.peek(RAM_OFFSET + i);
				if(b != previous[i]) {
					previous[i] = b;
					n_changes[i]++;
				}
			}
			ale.act(legal_actions[rand() % legal_actions.size()]);
		});

		cerr << "Reading frequency list\n";
		ofstream out(FREQUENCY_FILE);
		for(int i=0; i<RAM_SIZE; i++)
			out << i << ' ' << n_changes[i] << endl;
		out.close();
		frequency_file.open(FREQUENCY_FILE);
	}
	vector<pair<int, int> > freqs(RAM_SIZE);
	for(int i=0; i<RAM_SIZE; i++)
		frequency_file >> freqs[i].second >> freqs[i].first;
	sort(freqs.begin(), freqs.end());
	unique_ptr<vector<int> > result(new vector<int>(RAM_SIZE));
	for(int i=0; i<RAM_SIZE; i++)
		(*result)[i] = freqs[i].second;
	return move(result);
}

void build_level_graph(ALEInterface &ale, uInt16 addr, int level) {
	constexpr int SECONDS = 60*1;
	constexpr int MAX_STATE_VAL = 0x100;

	DirectedGraph dg;
	string graph_fname = "level_" + to_string(level) + ".graph";
	ifstream graph_file(graph_fname);
	if(!graph_file.is_open()) {
		cerr << "Building level " << level << " graph\n";
		AUX_VARIABLES
		unsigned n_actions = legal_actions.size();
		bool state_possible[MAX_STATE_VAL];
		// state x action -> state
		vector<int> transitions(MAX_STATE_VAL*n_actions);
		std::fill(transitions.begin(), transitions.end(), MAX_STATE_VAL);

		addr = RAM_OFFSET + addr;
		unsigned s = sys.peek(addr), ss = s;
		state_possible[s] = true;
		TIMED_RESETTING_LOOP({
			unsigned a = rand() % n_actions;
			ale.act(legal_actions[a]);
			ss = ale.getRAM().get(addr);
			ss = sys.peek(addr);
			unsigned i = s*n_actions + a;
			if(transitions[i] == MAX_STATE_VAL)
				transitions[i] = ss;
			else if(transitions[i] != ss)
				transitions[i] = -1;
			s = ss;
			state_possible[s] = true;
		});

		dg.adj_list = vector<vector<int> >(MAX_STATE_VAL);
		vector<unsigned> exits;
		for(int s=0; s<MAX_STATE_VAL; s++)
			for(int a=0; a<n_actions; a++) {
				int ss = transitions[s*n_actions + a];
				if(ss == -1)
					exits.push_back(s*n_actions + a);
				else if(ss != MAX_STATE_VAL)
					dg.adj_list[s].push_back(ss);
			}
		dg.save(graph_fname);
		cout << exits.size() << endl;
		for(size_t i=0; i<exits.size(); i++)
			cout << "Action: " << exits[i]%n_actions << ", state: " << exits[i]/n_actions << endl;
	}
	dg.load(graph_fname);
	dg.save_dot("altre.dot");
	vector<int> scc;
	int n_scc = dg.scc(scc);
	unique_ptr<DirectedGraph> dag = dg.assignment_merge(scc, n_scc);
	dag->save_dot("level_" + to_string(level) + ".dot");
}

int main(int argc, char *argv[]) {
	ALEInterface ale;

	ale.setInt("random_seed", 1234);
	ale.setBool("display_screen", false);
	ale.setBool("sound", false);
	ale.setInt("fragsize", 64);
	ale.setInt("frame_skip", 1);
	ale.loadROM(ROM_FILE);

	ActionVect legal_actions = ale.getLegalActionSet();

	unique_ptr<vector<int> > addr_by_freq = ram_addresses_by_change_frequency(ale);
	build_level_graph(ale, (*addr_by_freq)[0], 0+1);
	return 0;
}
