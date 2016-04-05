#ifndef MARKOV_DECISION_PROCESS_HPP
#define MARKOV_DECISION_PROCESS_HPP

#include <vector>

/*
 * These data types are not opaque. The interface of the MAXQLevel
 * class assumes they are integers. Still, the aliases are useful for clarity.
 */
typedef int Action;
typedef int State;
typedef double Reward;
/// Cartesian product: States x Actions
typedef int StateAction;

/** Interface for a HEXQ level. Conceptually it is also an MDP, but it does not
 * have quite the same interface because of implementation constraints.
 */
struct HexqLevelBase {
	/// Number of actions in the given state
	virtual Action n_actions(State s) const = 0;
	/// Number of possible states of this level
	virtual State n_states() const = 0;
	/// Current state of a given variable
	virtual State state() const = 0;
	/// Completely reset the state of the MDP
	virtual void Reset() = 0;
	/// Take the given action, modifying the state of the MDP
	virtual Reward TakeAction(Action a) = 0;
	/// Whether the MDP has terminated and a new episode should be started
	virtual bool terminated() const = 0;
};

/// Interface for an MDP with several discrete variables
class MarkovDecisionProcess : public HexqLevelBase {
protected:
	/// State of all the MDP variables
	std::vector<State> variables_;
	/// Given a variable give its frequency ranking
	std::vector<int> freq_variable_;
	/// Given a frequency ranking give its variable
	std::vector<int> variable_freq_;
	virtual State n_var_states_(int var) const = 0;
public:
	/// Number of variables for the MDP state
	size_t n_variables() const { return variables_.size(); }
	/// Current state of a given variable.
	/// Returns 1 for variable -1, to use for the first level of the HEXQ hierarchy
	State var_state(int var) const { return var == -1 ? 0 : variables_[var]; }
	/// Number of states for a given variable
	/// Returns 1 for variable -1, to use for the first level of the HEXQ hierarchy
	State n_var_states(int var) const { return var == -1 ? 1 : n_var_states_(var); }
	State *StateBuffer() const { return new State[n_variables()]; }
	// Copy state to the allocated buffer
	void FillStateBuffer(State *sb) const {
		for(int i=0; i<n_variables(); i++)
			sb[i] = variables_[i];
	}
	/** Has a variable or the ones less frequent than it changed between sa and
	 * sb?
	 */
	bool HasVarHierarchicalChanged(int var, State *sa, State *sb) const {
		// When the MDP level is the first one it has value -1
		if(var == -1) var = 0;
		int f = freq_variable_[var] + 1;
		for(; f<n_variables(); f++) {
			if(sa[variable_freq_[f]] != sb[variable_freq_[f]])
				return true;
		}
		return false;
	}

	State n_states() const { return n_var_states(0); }
	State state() const { return var_state(0); }
	MarkovDecisionProcess(int n_vars) : variables_(n_vars),
										freq_variable_(n_vars), variable_freq_(n_vars) {}
};

#endif // MARKOV_DECISION_PROCESS_HPP
