TODO
better error handling
logging
url parsing
concurrency (again)

Also would be nice if there was a better strategy for parsing / 
allocating static buffers for everything, not `malloc` but there's
probably a nicer solution.

we use lots of static buffer when string parsing, this gets kind of messy because
at any given moment there are a ton of static buffer lying around not being used,
wasting memory....
would be nice to have a single buffer where all string manipulation happens,
potentially make this its own library?
