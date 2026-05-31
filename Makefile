all:
	g++ -std=c++17 main.cpp Set.cpp Exercise.cpp Workout.cpp ProgressTracker.cpp ExerciseCatalog.cpp User.cpp Menus.cpp -o workout_tracker.exe -luser32 -lgdi32

