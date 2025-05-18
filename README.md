<p align="center">
  <img src="resources/JlangLogo.png" width="180" alt="Jlang Logo">
</p>

<p align="center">
  <em>A procedural programming language inspired by C and Go.</em>
</p>


## Langauge syntax ##

```Go
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

int32 main()
{
    var p Person* = (struct Person*) jalloc(sizeof(struct Person));

    if (p == NULL) 
    {
        jout("No can do"); 
    }
    else 
    {
        jout("Incredible");
    }

    jfree(p);
}
