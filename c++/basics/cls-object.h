
#ifndef CLSOBJECT_H
#define CLSOBJECT_H

class Time {
	private:
		int hour; // 0 to 23
		int min;  // 0 to 59
		int sec;  // 0 to 59
	public:
		Time(int h = 0, int m = 0, int s = 0);           // Constructor with default arguments
		int getHour() const;
		void setHour(int h);
		int getMin() const;
		void setMin(int m);
		int getSec() const;
		void setSec(int s);
		void setTime(int h, int m, int s);
		void print() const;

		Time& addTimeInto(const Time& time);              // Return by reference
		Time addTimeReturnNew(const Time& time) const;    // Return by value
};
#endif
