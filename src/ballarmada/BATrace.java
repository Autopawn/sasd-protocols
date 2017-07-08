package ballarmada;

import java.util.LinkedList;
import java.util.TreeSet;

public class BATrace{
	// Saves all the states from the begining of the game, so that it can warp.
	private LinkedList<BAState> states;
	private LinkedList<TreeSet<BAEvent>> events;
	private int init_time;

	int get_init_time(){
		return init_time;
	}

	public BATrace(int init_time, BAState init_state){
		states = new LinkedList<BAState>();
		states.add(init_state);
		events = new LinkedList<TreeSet<BAEvent>>();
		events.add(new TreeSet<BAEvent>());
		this.init_time = init_time;
	}

	private TreeSet<BAEvent> get_events_at(int time){
		int pos_at_events = init_time+events.size()-1-time;
		if(pos_at_events>=events.size()) return null;
		//^ There is no registered events, time is before initing.
		while(pos_at_events<0){
			events.addFirst(new TreeSet<BAEvent>());
			pos_at_events+=1;
		}
		return events.get(pos_at_events);
	}

	public BAState get_state_at(int time){
		if(time<init_time) return null;
		//^ time is before initing.
		int state_top = init_time+states.size()-1;
		while(state_top<time){
			// Add missing states until the one at time is reached:
			TreeSet<BAEvent> events_top = get_events_at(state_top);
			states.addFirst(states.get(0).update(events_top));
			state_top+=1;
		}
		int pos_at_states = init_time+states.size()-1-time;
		return states.get(pos_at_states).copy();
	}

	public boolean insert_event(int time, BAEvent event){
		TreeSet<BAEvent> events_at_time = get_events_at(time);
		if(events_at_time==null) return false;
		events_at_time.add(event);
		// Delete computed states after that event:
		int pos_at_states = init_time+states.size()-1-time;
		for(int k=0;k<pos_at_states;k++){
			states.removeFirst();
		}
		return true;
	}

	public void rebase_state(int init_time, BAState init_state){
		//^ Set a new initial state, keeping the events after it.
		states = new LinkedList<BAState>();
		states.add(init_state);
		// Recycle the events after the new init_time:
		LinkedList<TreeSet<BAEvent>> old_events = events;
		events = new LinkedList<TreeSet<BAEvent>>();
		int surviving_events = events.size()-(init_time-this.init_time);
		for(TreeSet<BAEvent> evs:old_events){
			if(surviving_events<=0) break;
			events.add(evs);
			surviving_events--;
		}
		while(surviving_events>0) events.add(new TreeSet<BAEvent>());
		if(events.size()==0) events.add(new TreeSet<BAEvent>());
		this.init_time = init_time;
	}
}
