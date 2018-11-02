# WIP

SoulHack 0.1

- hooks lua
- spawns console that can execute lua code
- hooks lua's print function to redirect lua output to console output
- prevent crash (WIP, working as VEH)
- prevent antidebug (WIP, I have no idea if that works)


** How to use **

- Update minhook (I removed binaries)
- Inject with your favourite injector (ManualMap + ThreadHijack)


** Problems **

- Game crashes after random command execution.
- Have to restart all threads in case of crash (and the threads on their own are overkill as I think :thinking:)


Credits:

* Based on someone's github repo (I will add link later, can't find that repo), but fixed all errors and improved.
