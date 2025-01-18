{* DALGOL24 the dynamically typed ALGOL! *}
program fibonacci
begin
    procedure fib2(curr, prev, count, max) 
    begin
        if (count == max) 
            begin
                return curr;
            end 
        else
            begin
                return fib2(curr+prev, curr, count+1, max);
            end
    end
    procedure fib(let k)
    begin
        if (k < 2) then 
            begin 
                return k; 
            end
        else
            begin 
                return fib(k-1) + fib(k-2); 
            end
    end
    let i := 1;
    while (i <= 50) {
        println i + ": " + fib2(0, 1, 1, i);
        i := i + 1;
    }

end.