ballarmada:
	cd src/java/ballarmada; rm *.class || true
	cd src/java/ballarmada; javac BAEvent.java
	cd src/java/ballarmada; javac BABall.java
	cd src/java/ballarmada; javac BAState.java
	cd src/java/ballarmada; javac BATrace.java
	cd src/java/ballarmada; javac BAGameplay.java
	cd src/java/ballarmada; javac BAGameTest.java
run:
	cd src/java/ballarmada; java BAGameTest
document:
	cd tex; pdflatex main.tex
	cd tex; bibtex main
	cd tex; pdflatex main.tex
	cd tex; pdflatex main.tex
	mv tex/main.pdf main.pdf
