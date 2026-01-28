import unreal
import random

def generate_forest(num_trees=100, area_size=2000):
    """
    Exemplo de script que a IA geraria para criar uma floresta procedural.
    Este script deve ser enviado para POST /api/run_script
    """
    print(f"[LogosBridge] Gerando floresta com {num_trees} arvores...")
    
    # Carregar asset (exemplo)
    # Na pratica, a IA teria usado /api/query para achar o caminho correto antes
    tree_path = "/Engine/BasicShapes/Cone" 
    
    with unreal.ScopedEditorTransaction("LogosBridge Forest Gen"):
        for i in range(num_trees):
            x = random.uniform(-area_size, area_size)
            y = random.uniform(-area_size, area_size)
            z = 0 # Raycast seria ideal aqui
            
            location = unreal.Vector(x, y, z)
            rotation = unreal.Rotator(0, random.uniform(0, 360), 0)
            
            # Spawn
            actor = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.StaticMeshActor, location, rotation)
            
            if actor:
                actor.set_actor_label(f"ProcTree_{i}")
                actor.set_actor_scale3d(unreal.Vector(1, 1, random.uniform(0.8, 1.5)))
                print(f"Spawned {actor.get_actor_label()}")

    print("[LogosBridge] Geracao concluida!")

if __name__ == "__main__":
    generate_forest()
