program scoping
    let a := "global";
    def showA() {
        println a;
    }
    def ok() {
        let a := "inner";
        showA();
        println a;
    }
    showA();
    ok();
    showA();
end.