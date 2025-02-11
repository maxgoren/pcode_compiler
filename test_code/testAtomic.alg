program testatomic
begin
    let glob := 0;
    procedure testAtomic(n)
    begin
        let i := 0;
        while (i < 100) do
        begin
            glob := glob + 1;
            glob := glob + (i + 1);
            i := i + 1;
        end;
        return glob;
    end
    println testAtomic(15);
end.