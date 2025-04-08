#!/usr/local/bin/dalgol -v
program fibonacci
begin
    procedure fibo(var n)
    begin
        if (n <= 1) then
        begin
            return 1;
        end
            else
        begin
            return fibo(n-2)+fibo(n-1);
        end
    end
    println fibo(6);
end.