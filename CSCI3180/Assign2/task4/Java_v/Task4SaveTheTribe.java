import java.util.Random;
import java.util.Scanner;

public class Task4SaveTheTribe {
  private Task4Map map;
  private Task4Soldier soldier;
  private Spring spring;
  private Task4Monster[] monsters;
  private Merchant merchant;
  
  private boolean gameEnabled;
  
  public Task4SaveTheTribe() {
    this.map = new Task4Map();
    this.soldier = new Task4Soldier();
    this.spring = new Spring();
    this.monsters = new Task4Monster[7];
    this.merchant = new Merchant();
    this.gameEnabled = true;
  }


public void initialize() {
    /* We use the integer 1-7 to represent the keys for corresponding caves, and the integer -1 to represent the artifact. */

    Random random = new Random();

    this.monsters[0] = new Task4Monster(1, random.nextInt(5) * 10 + 30);
    this.monsters[0].setPos(4, 1);
    this.monsters[0].addDropItem(2);
    this.monsters[0].addDropItem(3);
 
    this.monsters[1] = new Task4Monster(2, random.nextInt(5) * 10 + 30);
    this.monsters[1].setPos(3, 3);
    this.monsters[1].addDropItem(3);
    this.monsters[1].addDropItem(6);
    this.monsters[1].addHint(1);
    this.monsters[1].addHint(5);

    this.monsters[2] = new Task4Monster(3, random.nextInt(5) * 10 + 30);
    this.monsters[2].setPos(5, 3);
    this.monsters[2].addDropItem(4);
    this.monsters[2].addHint(1);
    this.monsters[2].addHint(2);

    this.monsters[3] = new Task4Monster(4, random.nextInt(5) * 10 + 30);
    this.monsters[3].setPos(5, 5);
    this.monsters[3].addHint(3);
    this.monsters[3].addHint(6);

    this.monsters[4] = new Task4Monster(5, random.nextInt(5) * 10 + 30);
    this.monsters[4].setPos(1, 4);
    this.monsters[4].addDropItem(2);
    this.monsters[4].addDropItem(6);

    this.monsters[5] = new Task4Monster(6, random.nextInt(5) * 10 + 30);
    this.monsters[5].setPos(3, 5);
    this.monsters[5].addDropItem(4);
    this.monsters[5].addDropItem(7);
    this.monsters[5].addHint(2);
    this.monsters[5].addHint(5);

    this.monsters[6] = new Task4Monster(7, random.nextInt(5) * 10 + 30);
    this.monsters[6].setPos(4, 7);
    this.monsters[6].addDropItem(-1);
    this.monsters[6].addHint(6);

    this.map.addObject(monsters);

    this.soldier.setPos(1, 1);
    this.soldier.addKey(1);
    this.soldier.addKey(5);

    this.map.addObject(this.soldier);

    this.spring.setPos(7, 4);

    this.map.addObject(this.spring);
    
    this.merchant.setPos(7, 7);
    
    this.map.addObject(this.merchant);
  }

  public void start() {
    System.out.printf("=> Welcome to the desert!%n");
    System.out.printf("=> Now you have to defeat the monsters and find the artifact to save the tribe.%n%n");

    Scanner sc = new Scanner(System.in);

    while (gameEnabled) {
      this.map.displayMap();
      this.soldier.displayInformation();

      System.out.printf("%n=> What is the next step? (W = Up, S = Down, A = Left, D = Right.) Input: ");
      
      String move = sc.nextLine();

      Pos pos = this.soldier.getPos();
      int newRow, oldRow;
      int newColumn, oldColumn;

      newRow = oldRow = pos.getRow();
      newColumn = oldColumn = pos.getColumn();

      if (move.equalsIgnoreCase("W")) {
        newRow = oldRow - 1;
      } else if (move.equalsIgnoreCase("S")) {
        newRow = oldRow + 1;
      } else if(move.equalsIgnoreCase("A")) {
        newColumn = oldColumn - 1;
      } else if(move.equalsIgnoreCase("D")) {
        newColumn = oldColumn + 1;
      } else {
        System.out.printf("=> Illegal move!%n%n");
        continue;
      }

      if (this.map.checkMove(newRow, newColumn)) {
        Object occupiedObject = this.map.getOccupiedObject(newRow, newColumn);

        if (occupiedObject instanceof Task4Monster) {
          ((Task4Monster)occupiedObject).actionOnSoldier(this.soldier);
        } else if (occupiedObject instanceof Spring) {
          ((Spring)occupiedObject).actionOnSoldier(this.soldier);
        } else if (occupiedObject instanceof Merchant) {
            ((Merchant)occupiedObject).actionOnSoldier(this.soldier);
        } else {
          this.soldier.move(newRow, newColumn);
          this.map.update(this.soldier, oldRow, oldColumn, newRow, newColumn);
          System.out.printf("%n%n");
        }
      } else {
        System.out.printf("=> Illegal move!%n%n");
      }

      if (this.soldier.getHealth() <= 0) {
        System.out.printf("=> You died.%n");
        System.out.printf("=> Game over.%n%n");
        this.gameEnabled = false;
      }

      /* Check if the soldier has received the artifact. */
      if (this.soldier.getKeys().contains(-1)) {
      	System.out.printf("=> You found the artifact.%n");
        System.out.printf("=> Game over.%n%n");
      	this.gameEnabled = false;
      }
    }
  }

  public static void main(String[] args) {
    Task4SaveTheTribe game = new Task4SaveTheTribe();
    game.initialize();
    game.start();
  }
}