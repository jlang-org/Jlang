interface IPrintable
{
    void print();
}

struct Person -> IPrintable 
{
    firstName char*; 
    age int32; 
}

void print() -> Person p
{
    jout("First name: %s", p.firstName); 
    jout("Age: %d", p.age); 
}

struct Frog -> IPrintable 
{
    name char*; 
    age int32; 
}

void print() -> Frog f
{
    jout("Frogs name: %s", f.name); 
    jout("Age: %d", f.age); 
}

int32 main()
{
    var person Person* = (struct Person*) jalloc(sizeof(struct Person));

    if (person == NULL) 
    {
        jout("No can do"); 
    }
    else 
    {
        jout("Incredible");
    }

    jfree(person);
}
