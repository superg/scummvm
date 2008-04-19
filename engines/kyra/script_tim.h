/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef KYRA_SCRIPT_TIM_H
#define KYRA_SCRIPT_TIM_H

#include "kyra/kyra.h"
#include "kyra/util.h"

#include "common/array.h"

namespace Kyra {

struct TIM {
	int16 procFunc;
	uint16 unkFlag;

	struct Function {
		const uint16 *ip;

		uint32 lastTime;
		uint32 nextTime;

		const uint16 *avtl;
	} func[10];

	uint16 *avtl;
	uint8 *text;

	const Common::Array<const TIMOpcode*> *opcodes;
};

class TIMInterpreter {
public:
	TIMInterpreter(KyraEngine *vm, OSystem *system);

	TIM *load(const char *filename, const Common::Array<const TIMOpcode*> *opcodes);
	void unload(TIM *&tim) const;

	void resetFinishedFlag() { _finished = false; }
	bool finished() const { return _finished; }

	void exec(TIM *tim, bool loop);
	void stopCurFunc() { if (_currentTim) cmd_stopCurFunc(0); }

	void play(const char *filename);
private:
	KyraEngine *_vm;
	OSystem *_system;

	TIM *_currentTim;
	int _currentFunc;

	bool _finished;
	
	int execCommand(int cmd, const uint16 *param);

	typedef int (TIMInterpreter::*CommandProc)(const uint16 *);
	struct CommandEntry {
		CommandProc proc;
		const char *desc;
	};

	const CommandEntry *_commands;
	int _commandsSize;

	int cmd_restartFunc0(const uint16 *param);
	int cmd_stopCurFunc(const uint16 *param);
	int cmd_initFunc(const uint16 *param);
	int cmd_stopFunc(const uint16 *param);
	int cmd_resetAllNextTime(const uint16 *param);
	int cmd_execOpcode(const uint16 *param);
	int cmd_initFuncNow(const uint16 *param);
	int cmd_stopFuncNow(const uint16 *param);
	template<int T>
	int cmd_return(const uint16 *) { return T; }
};

} // end of namespace Kyra

#endif

