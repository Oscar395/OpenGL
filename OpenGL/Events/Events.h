#pragma once
#include <GLFW/glfw3.h>

namespace ForByte {
	class Events {
	public:
		~Events();
		Events(const Events&) = delete;
		Events& operator=(const Events&) = delete;

		static Events& Ref() {
			static Events reference;
			return reference;
		}

		void Poll();
		void Initialize();

	private:
		Events();

	private:

	};

	static Events& Event = Events::Ref();
}