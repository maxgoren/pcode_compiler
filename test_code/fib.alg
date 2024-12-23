{* DALGOL24 the dynamically typed ALGOL! *}
program fibonacci
begin
    procedure fib(let k)
    begin
        if (k < 2) then 
            begin 
                return 1; 
            end
        else
            begin 
                return fib(k-1) + fib(k-2); 
            end
    end
    println fib(40);
end.