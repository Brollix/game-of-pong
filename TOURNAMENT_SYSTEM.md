# AI Tournament System - Implementation Documentation

## Overview

This document describes the comprehensive AI vs AI tournament system with evolutionary population that has been implemented in the Pong game. The system allows multiple AI agents to compete against each other, evolve through genetic algorithms, and improve their strategies over generations.

## Architecture

### Component Hierarchy

```
Game.h
  └─ TournamentManager.h
      ├─ AIPopulation.h
      │   └─ AIIndividual (wraps AIPlayer)
      └─ TournamentMatch.h
          ├─ AIPlayer.h
          ├─ Ball.h
          └─ Collisions.h
```

### New Files Created

1. **`src/AIPopulation.h`** (~380 lines)

   - `GeneticParams`: Structure for genetic parameters
   - `AIIndividual`: Wrapper for AIPlayer with fitness tracking
   - `AIPopulation`: Population management with genetic operations

2. **`src/TournamentMatch.h`** (~230 lines)

   - `MatchResult`: Structure for match results
   - `TournamentMatch`: Headless and visual match simulator

3. **`src/TournamentManager.h`** (~450 lines)
   - `TournamentConfig`: Configuration parameters
   - `GenerationStats`: Statistics tracking per generation
   - `TournamentManager`: Orchestrates tournaments and evolution

### Modified Files

1. **`src/NeuralNetwork.h`**

   - Added constructor with custom hidden layer size
   - Added `copyTo()` method for cloning without move semantics issues
   - Added `getLayerSizes()` getter

2. **`src/QLearningAgent.h`**

   - Added constructor with custom hyperparameters
   - Added getters/setters for all hyperparameters
   - Added `copyTo()` method for cloning
   - Removed duplicate `setLearningRate()` method

3. **`src/AIPlayer.h`**

   - Added constructor with custom hyperparameters
   - Added `clone()` method
   - Added getters for all genetic parameters

4. **`src/HUD.h`**

   - Added `renderTournamentStats()` for tournament progress
   - Added `renderTopIndividuals()` template method for rankings

5. **`src/Game.h`**
   - Added `Tournament` and `TournamentMenu` game states
   - Added `TournamentManager*` member
   - Added tournament menu with options
   - Added tournament rendering and update logic
   - Added keyboard controls for tournament mode

## Genetic Algorithm Details

### Genetic Parameters

Each AI individual has 5 genetic parameters that control its learning:

| Parameter             | Range          | Description                                 |
| --------------------- | -------------- | ------------------------------------------- |
| **Learning Rate**     | 0.001 - 0.05   | How fast the neural network learns          |
| **Epsilon Decay**     | 0.95 - 0.995   | Rate of exploration→exploitation transition |
| **Hidden Layer Size** | 8 - 24 neurons | Neural network capacity                     |
| **Discount Factor**   | 0.90 - 0.99    | Importance of future rewards                |
| **Batch Size**        | 16 - 64        | Samples per training batch                  |

### Evolution Process

```
Generation 0:
  1. Initialize population with random genetic parameters
  2. Each AI starts with randomly initialized neural network

For each generation (1 to MAX_GENERATIONS):
  1. TOURNAMENT PHASE
     - Round-Robin: Each AI plays every other AI twice (home & away)
     - Record wins, losses, scores

  2. EVALUATION PHASE
     - Calculate fitness: fitness = (win_rate * 0.7) + (agent_fitness * 0.3)
     - Sort population by fitness (descending)

  3. SELECTION PHASE
     - Elite selection: Top 25% survive directly
     - Rest are discarded

  4. REPRODUCTION PHASE
     - For each offspring slot:
       - Select two random elite parents
       - Crossover: Randomly choose each gene from either parent
       - Mutate: 10% chance per gene to vary by ±20%

  5. SAVE PHASE
     - Save top 3 models to disk
     - Save population state
```

### Fitness Calculation

Fitness is calculated as:

```
fitness = (tournament_win_rate * 0.7) + (agent_internal_fitness * 0.3)
```

Where:

- **tournament_win_rate**: Wins / Total matches in current generation
- **agent_internal_fitness**: (recent_win_rate _ 0.7) + ((1.0 - epsilon) _ 0.3)

This dual fitness approach ensures:

1. Tournament performance matters most (70%)
2. Internal learning quality also counts (30%)
3. Lower epsilon (more exploitation) = better learned strategy

## Configuration Options

### Tournament Configuration

```cpp
struct TournamentConfig {
    int populationSize;       // 8, 16, 32 (default: 16)
    int maxGenerations;       // 10, 50, 100, ∞ (default: 50)
    int pointsPerMatch;       // 3, 5, 7 (default: 7)
    float speedMultiplier;    // 1x, 10x, 100x (default: 10x)
    float elitePercent;       // 0.25 = 25% elite (default)
    float mutationRate;       // 0.1 = 10% mutation (default)
    TournamentMode mode;      // RoundRobin, Swiss, Evolutionary
};
```

### Crossover and Mutation

**Crossover (Uniform Crossover):**

- For each genetic parameter, randomly select from either parent with 50% probability
- Example: Child might get learning rate from parent1, epsilon decay from parent2, etc.

**Mutation:**

- Each gene has 10% chance to mutate
- Mutation range: parameter \* (0.8 to 1.2)
- Integer parameters (hidden size, batch size) use discrete mutations (±4, ±8)
- All mutations are clamped to valid parameter ranges

## File System Organization

```
models/
  ├── generation_001/
  │   ├── rank_1_id_5_fitness_0.850.bin
  │   ├── rank_1_id_5_metrics.txt
  │   ├── rank_2_id_12_fitness_0.780.bin
  │   ├── rank_2_id_12_metrics.txt
  │   └── rank_3_id_8_fitness_0.720.bin
  │       rank_3_id_8_metrics.txt
  ├── generation_002/
  │   └── ...
  ├── final_population.dat
  └── tournament_summary.txt
```

### Model Files

**Neural Network Models** (`*.bin`):

- Binary format with weights and biases
- Can be loaded into any AIPlayer for gameplay
- Compatible with existing ModelSaver system

**Metrics Files** (`*_metrics.txt`):

```
Individual ID: 5
Generation: 1
Rank: 1
Fitness: 0.8500
Win Rate: 0.9231
Wins: 12
Losses: 1
Total Matches: 13

Genetic Parameters:
  Learning Rate: 0.0234
  Epsilon Decay: 0.9721
  Hidden Layer Size: 16
  Discount Factor: 0.9567
  Batch Size: 32
```

## User Interface

### Main Menu

- **Start Game**: Human vs AI (existing)
- **Tournament**: Enter tournament mode (NEW)
- **Options**: Game configuration
- **Quit**: Exit game

### Tournament Menu

- **Start Tournament**: Begin evolution process
- **Configure**: Adjust tournament parameters (TODO)
- **View Stats**: See detailed statistics (TODO)
- **Watch Top Match**: Observe top 2 AIs competing (functional)
- **Back**: Return to main menu

### Tournament Screen

**Display Elements:**

1. **Progress Bar**

   - Shows current generation / max generations
   - Percentage completion
   - Status message (Running, Paused, Completed)

2. **Fitness Statistics**

   - Average fitness across population
   - Best fitness in current generation
   - Generation number

3. **Top 5 Leaderboard**

   - Rank (Gold/Silver/Bronze colors)
   - Individual ID
   - Fitness score
   - Win-Loss record
   - Win rate percentage

4. **Controls**
   - `SPACE`: Pause/Resume tournament
   - `ESC`: Return to tournament menu

## Match System

### Headless Matches (Fast Training)

```cpp
TournamentMatch match(1280, 720, 7, false); // No window
match.setupMatch(player1, player2);
MatchResult result = match.runHeadless(id1, id2, 10.0f); // 10x speed
```

- No rendering overhead
- Speed multiplier for faster training
- Deterministic results
- Full collision and scoring logic

### Visual Matches (Observation)

```cpp
TournamentMatch match(1280, 720, 7, true); // With window
match.setupMatch(player1, player2);
MatchResult result = match.runVisual(id1, id2); // Real-time
```

- Full rendering at 60 FPS
- Watch AI strategies
- ESC to close and return
- Useful for analyzing behavior

## Usage Examples

### Running a Full Tournament

```cpp
// In Game.h, when user selects "Start Tournament"
tournamentManager->initialize();
tournamentManager->start();
state = GameState::Tournament;

// Tournament runs automatically, one generation per frame
// When complete, returns to TournamentMenu
```

### Watching Specific Match

```cpp
// Watch champion vs runner-up
tournamentManager->watchMatch(0, 1); // indices in sorted population
```

### Loading Best Model for Play

```cpp
// Load champion from generation 50
ModelSaver::loadModel(
    opponent->getAgent().getNetwork(),
    "models/generation_050/rank_1_id_234_fitness_0.923.bin"
);
```

## Performance Characteristics

### Computational Complexity

**Per Generation:**

- Matches: N × (N-1) where N = population size
- Example: 16 population = 240 matches/generation
- With 10x speed: ~30-60 seconds/generation on modern hardware

**Full Tournament:**

- 50 generations × 240 matches = 12,000 matches
- Estimated time: 25-50 minutes at 10x speed
- Can pause/resume anytime

### Memory Usage

- Each AIIndividual: ~50KB (neural network + metadata)
- Population of 16: ~800KB
- Match history: Negligible
- Total: <5MB additional RAM

## Implementation Notes

### Design Decisions

1. **No Move Semantics for Cloning**

   - `random_device` cannot be copied or moved
   - Solution: `copyTo()` methods instead of `clone()` returning by value
   - Safer and more explicit

2. **Template for HUD Rendering**

   - `renderTopIndividuals()` uses template to avoid forward declaration issues
   - Works with any type that has `id`, `fitness`, `wins`, `losses`, `winRate`

3. **Training Disabled During Tournaments**

   - Ensures fair comparison
   - Prevents learning during evaluation
   - Training state restored after matches

4. **Binary File Format**
   - Cross-platform (Windows, Linux, Mac)
   - Compatible with existing ModelSaver
   - Simple to extend

### Known Limitations

1. **Configuration UI**: Currently only accessible via code (TODO: Add config menu)
2. **Statistics Visualization**: Text-only (TODO: Add graphs)
3. **Multi-threading**: Single-threaded evolution (could parallelize matches)
4. **Checkpointing**: Manual only (could auto-save every N generations)

### Future Enhancements

Potential improvements:

- [ ] Swiss tournament pairing system
- [ ] Neural architecture search (evolve network topology)
- [ ] Co-evolution with different game modes
- [ ] Multi-objective optimization (win rate + style metrics)
- [ ] Transfer learning from previous tournaments
- [ ] Graphical fitness plots over generations
- [ ] Real-time tournament broadcasting (spectator mode)
- [ ] Hyperparameter importance analysis

## Technical Details

### Thread Safety

- **Single-threaded**: All operations run on main game thread
- **No concurrency**: Headless matches are sequential
- **Safe for parallel future**: Population individuals are independent

### Error Handling

- Invalid indices checked before match setup
- File I/O errors logged to console
- Empty populations handled gracefully
- Malformed save files cause re-initialization

### Compatibility

- **C++ Standard**: C++17
- **SFML Version**: 2.6.2
- **Compiler**: GCC 15.2.0+ (MinGW), Clang 12+, MSVC 2019+
- **Platforms**: Windows (tested), Linux, macOS (should work)

## Conclusion

The tournament system provides a complete framework for:

1. ✅ Evolving AI populations through competition
2. ✅ Tracking performance across generations
3. ✅ Saving and loading best models
4. ✅ Visualizing tournament progress
5. ✅ Analyzing genetic parameter effectiveness

The system is extensible, well-documented, and integrates seamlessly with the existing Pong game architecture.

## Quick Start

1. **Compile**: `g++ -std=c++17 -ISFML-2.6.2/include main.cpp -o bin/pong.exe -LSFML-2.6.2/lib -lsfml-graphics -lsfml-window -lsfml-system -lopengl32 -lwinmm -lgdi32`

2. **Run**: `./bin/pong.exe`

3. **Start Tournament**:

   - Main Menu → Tournament
   - Start Tournament
   - Wait for evolution (SPACE to pause, ESC to exit)

4. **View Results**:
   - Check `models/` directory for saved champions
   - Load best model: Options → Load Model
   - Play against evolved AI!

---

**Created**: December 19, 2025  
**Version**: 1.0  
**Author**: AI Tournament System Implementation

