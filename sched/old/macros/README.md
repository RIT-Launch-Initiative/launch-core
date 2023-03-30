Combined with the scheduler, these macros allow us to make functions reentrant.

## Usage pattern
```
RetType example_function(...) { // must be RetType
	RESUME(); // this function is reentrant 
	// If a function CALLs, SLEEPs, BLOCKs, or YIELDs, it can jump back and
	// forth in execution and is reentrant. Such functions MUST have RESUME/RESET
	// pairs.
	RetType status;

	status = CALL(blocking_function(...));

	if (status == RET_ERROR) { // blocking_function failed for some reason
		// In this example, we elect not to continue execution
		// Therefore, we want the RESUME() macro to start this function from the beginning
		// next time this function is called.
		RESET(); // We do this using the RESET macro
  
		return RET_ERROR; // Return an error so the caller knows we failed
	}

	// interesting things...

	RESET(); // we are done, reset the current label to the start
	// Not needed if we don't use RESUME
	return RET_SUCCESS; // :)
}
```

Notes:
- All tasks are functions; not all functions are tasks. Calling reentrant
  *functions* DOES NOT create new tasks, only `sched_start` creates new tasks.
- If a task `SLEEP`s, `BLOCK`s, or `YIELD`s, it is reentrant and must have
  reentrant macros.

Common mistakes (to be updated as they happen):
- Not using `CALL` on functions that are reentrant (have `RESUME/RESET` macros
  inside them)
- Not using `RESUME`/`RESET` in functions that rely on other reentrant functions
- Failing to call `RESET` right before an early `return` (like for an error
status). The consequence of this is that the next call, this function will not
start from the beginning, and instead pick up from the last `CALL`, `SLEEP`,
`BLOCK`, or `YIELD`, which is probably not what you want.

## RESUME
**Sets up a function to be reentrant.**

### Summary
- Initializes the array of labels a function uses to know where it is, for
every task that may have called this function.
- Gives the start label.
- Jumps to where the currently executing task should be in this function

### All at once
```
#define RESUME()
static bool _init = false;
static void* _current[static_cast<int>(MAX_NUM_TASKS) + 1];
if(!_init) {
	for(int i = 0; i < static_cast<int>(MAX_NUM_TASKS) + 1; i++) {
		_current[i] = &&_start;
	}
	_init = true;
}
goto *(_current[static_cast<int>(sched_dispatched)]);
_start:
```
Several questions pop out:
- What does `_init` do?
- What stuff does each element of `_current` point to?
- Why does `_current` need to be an array?
- What is `sched_dispatched?`

### Stepping through
`_init` is static to make sure the array of labels `_current` only gets
initialized once - starts out false:
```
static bool _init = false;
```

`_current` is an array of labels, one larger than the maximum number of tasks:
```
static void* _current[static_cast<int>(MAX_NUM_TASKS) + 1];
```
`_current` must be an array because any of the scheduled tasks (of which there are at
most `MAX_NUM + 1`) may call this re-entrant function, and will need its own
re-entry label. 

Initialize (if it hasn't been already - this is what `_init` controls) all of
the labels in the `_current` array to the start label:
```
if(!_init) {
	for(int i = 0; i < static_cast<int>(MAX_NUM_TASKS) + 1; i++) {
		_current[i] = &&_start;
	}
	_init = true;
}
```
NOTE: The unary `&&` operator lets us store the label address as a variable and is 
[GCC specific](http://gcc.gnu.org/onlinedocs/gcc-3.2.3/gcc/Labels-as-Values.html#Labels%20as%20Values).

Skip ahead to wherever the current task needs this function to be:
```
goto *(_current[static_cast<int>(sched_dispatched)]);
```
`sched_dispatched` is the ID (`tid_t`) of the currently executing task. It is
kept up to date by the scheduler functions - DO NOT MODIFY!

Label the start of the function:
```
_start:
```

## RESET
Sets this (as given by `sched_dispatched`) task's position in this function to
the start.
```
#define RESET() _current[static_cast<int>(sched_dispatched)] = &&_start;
```
This means any time you want a function to start from the beginning the next
time the *same* task calls this function, `RESET` should happen immediately
before that return. This would be in whatever block has a `RET_SUCCESS` or
`RET_ERROR`, which are the return values that correspond to "I'm done, see you at
the beginning next time". (if you have an `if (bad_thing) return RET_ERROR;`,
`RESET()` should occur immediately before `return RET_ERROR`)

## CALL
**Calls a reentrant function**

### Summary
- Behaves appropriately according to the call's return value: Success or error entail continuing with execution and handling the results (the tailing `RET;`), and blocking, sleeping, or yielding entail exiting immediately (`return RET`).
- Sets the `_current` label to right before the function call so that the scheduler can jump back here and re-execute the call once the task wakes up or unblocks.

### All at once
```
#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)


#define CALL2(F, RET, z)
    ({
	    _current[static_cast<int>(sched_dispatched)] = TOKENPASTE2(&&_call, z); 
	    TOKENPASTE2(_call, z):; 
	    RetType RET = F; 
	    if(RET == RET_SLEEP || RET == RET_BLOCKED || RET == RET_YIELD) {return RET;}; 
	    RET;
	})\

/// ...
#define CALL(F) CALL2(F, TOKENPASTE2(__ret, __COUNTER__), __COUNTER__)
```
Immediate questions:
- Why is `##` wrapped twice using `TOKENPASTE(2)`?
- Why is `CALL2` wrapped in `CALL` instead of just putting those macro expansions as they are?
- What is `__COUNTER__`?

### Stepping through
For now we'll ignore the nested calls. The property we care about is that these
`CALLS` get a unique variable to store the result (given to us by the
GCC-specific `__COUNTER__` macro) and more importantly a unique label to jump
back to. We will for now just assume that `RET` and `z` are unique to each
`CALL`.

Store the label right before the function call as the current position (`z` is
a unique identifier for this `CALL`):
```
_current[static_cast<int>(sched_dispatched)] = TOKENPASTE2(&&_call, z); 
```

Drop a label right before the call to `F`:
```
TOKENPASTE2(_call, z):; 
```

This label should occur *before* `F` and not after because we want to
re-execute `F` if this `CALL` returns something that indicates we need to
resume `F` later.

Execute `F` and store its results in `RET` (which is a unique variable name to
this `CALL` thanks to fuckery):
```
RetType RET = F; 
```

These status codes are variations on "try again later", so in such cases we
immediately return from this function: and we stored the label right before the
`F` call ):
```
if(RET == RET_SLEEP || RET == RET_BLOCKED || RET == RET_YIELD) {return RET;}; 
```
We stored the label right before the `F` call, so we will jump here and call
`F` (and thanks to `RESUME`, pick up from where we left off) next time we call
this function.

If it isn't a "try again later status", drop it here so the calling block knows
what happened (`RET_SUCCESS` or `RET_ERROR`):
```
RET;
```

## SLEEP
### All at once
```
#define SLEEP2(N, z)
_current[static_cast<int>(sched_dispatched)] = TOKENPASTE2(&&_sleep, z);
sched_sleep(sched_dispatched, N);
return RET_SLEEP;
TOKENPASTE2(_sleep, z):

#define SLEEP(N) SLEEP2(N, __COUNTER__)
```
`CALL` is probably the most complicated macro, so there shouldn't be new
questions here. (as before, we ignore the nested calling for now)

### Stepping through
Use `_current` to store this unique SLEEP point (`z`), for this (`sched_dispatched`) task:
```
_current[static_cast<int>(sched_dispatched)] = TOKENPASTE2(&&_sleep, z);
```

Tell the scheduler to put this on the sleep queue for however many ticks (N) we want:
```
sched_sleep(sched_dispatched, N);
```

Return from the function (and every caller above will immediately return as well):
```
return RET_SLEEP;
```

Put this invocation's unique label here:
```
TOKENPASTE2(_sleep, z):
```

Once this task goes back on the ready queue (the sleep time expires), the
`goto`s inside the `RESUME` expansions will send us here.  This goes after the
return so we don't immediately go to sleep when we wake back up.

## BLOCK
```
#define BLOCK2(z)
_current[static_cast<int>(sched_dispatched)] = TOKENPASTE2(&&_block, z);
sched_block(sched_dispatched);
return RET_BLOCKED;
TOKENPASTE2(_block, z):

#define BLOCK() BLOCK2(__COUNTER__)

```
Similar to `SLEEP`, but uses `sched_block`. Something else (an ISR callback or
`poll`) unblocks this task, and the `RESUME` expansions go down the chain of
`gotos` to get back to where we need to be.

## YIELD
```
#define YIELD2(z)
_current[static_cast<int>(sched_dispatched)] = TOKENPASTE2(&&_yield, z);
return RET_YIELD;
TOKENPASTE2(_yield, z):

#define YIELD() YIELD2(__COUNTER__)
```
`SLEEP`, but will immediately re-run once the scheduler goes through everything else.

## WAKE
Wraps `sched_wake`. This is the only macro for which you need to know a task's
ID. In our use, such an ID would be stored in some variable by a task right
beforte that task blocks (see the [HAL I2C Device](../../device/platforms/stm32/HAL_I2CDevice.h)).
