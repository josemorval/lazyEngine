#pragma once
#ifndef _LIBMIDI_H_
#define _LIBMIDI_H_
#endif

namespace libmidi
{ 
	const int MAX_KEYS = 16;
	const int MAX_SUBSCRIPTIONS = 5;
    const int MAX_MIDI_MESSAGES = 10;
    UINT midiMessages[MAX_MIDI_MESSAGES];
	UINT debugMidiMessages[MAX_MIDI_MESSAGES];

	void (*eval_key[MAX_KEYS][MAX_SUBSCRIPTIONS])(int,float) = { nullptr };

	void CALLBACK midiin_callback(HMIDIIN _hMidiIn, UINT _wMsg, DWORD_PTR _dwInstance, DWORD_PTR _dwParam1, DWORD_PTR _dwParam2) 
	{
		for (int i = MAX_MIDI_MESSAGES - 1; i > 0; i--)
		{
			midiMessages[i] = midiMessages[i - 1];
			debugMidiMessages[i] = debugMidiMessages[i - 1];
		}

		midiMessages[0] = _dwParam1;
		debugMidiMessages[0] = _dwParam1;
	}

	void setup_device()
	{
		HMIDIIN hmidiin;
		MMRESULT result = midiInOpen(&hmidiin, 0, (DWORD_PTR)(&midiin_callback), 0, CALLBACK_FUNCTION);

		result = midiInStart(hmidiin);
	}

	void consume_inputs()
	{
		for (int i = MAX_MIDI_MESSAGES-1; i>-1; i--) {
			UINT midiMessage = midiMessages[i];

			if (midiMessages[i] != 0xFFFFFFFF)
			{
				int channel = (char)((midiMessage >> 0) & 0xF);
				int index = (int)((midiMessage >> 8) & 0xFF);
				int val = (char)((midiMessage >> 16) & 0xFF);
				float fval = val / 127.0;

				for (int i = 0; i < 5; ++i)
				{
					if (eval_key[index][i] != nullptr)
					{
						eval_key[index][i](index, fval);
					}
				}
			}
		}

		for (int i = 0; i < MAX_MIDI_MESSAGES; ++i) 
		{
			midiMessages[i] = 0xFFFFFFFF;
		}
	}

	void subscribe_key(int key, void (*func)(int index, float val))
	{
		for (int i = 0; i < 5; ++i) {
			if (eval_key[key][i] == nullptr) {
				eval_key[key][i] = func;
				break;
			}
		}
	}

	void debug_inputs()
	{
		for (int i = MAX_MIDI_MESSAGES - 1; i > -1; i--)
		{
			UINT midiMessage = debugMidiMessages[i];

			int channel = (char)((midiMessage >> 0) & 0xF);
			int index = (int)((midiMessage >> 8) & 0xFF);
			int val = (char)((midiMessage >> 16) & 0xFF);
			float fval = val / 127.0;

			ImGui::Text("Index %d, Val %d, fval %f\n", index, val, fval);
		}
	}

}
