program nesting
begin
    let q := 10;
    procedure A(k) 
    begin
        let j := (2*k-1);
        procedure B(m)
        begin
            println "B: " + m;
            return A(m+j)/3;
        end
        println "A: " + j;
        if (j < 100) then 
            begin
               j := A(j+1);
            end
        else 
            begin
                j := B(j/3-1);
            end
    end
    
    A(1);
end.