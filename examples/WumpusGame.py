import os

def generate_final_specs_and_partition():
    root_path = ""


    # # Gioco 2 x 2
    # rows, cols = 2, 2
    # gold_positions = [(0, 0)]
    # wumpus_start = (1, 1)
    # agent_start = (1, 0)
    # destination = (1, 0)

    # Gioco 3 x 3
    rows, cols = 3, 3
    gold_positions = [(2, 0),(1, 1), (0, 2),(0, 0)]
    wumpus1_start = (0, 2)
    wumpus2_start = (2, 2)
    agent_start = (1, 0)
    

    #4 x 4 game
    # rows, cols = 4, 4
    # gold_positions = [(0, 1), (1, 2), (3, 2)]
    # wumpus_start = (2, 3)
    # agent_start = (3, 0)
    # destination = (0, 0)
  
    agent_actions = ["anorth", "asouth", "aeast", "awest"]
    wumpus1_actions = ["w1north", "w1south", "w1east", "w1west"]
    wumpus2_actions = ["w2north", "w2south", "w2east", "w2west"]
    
  
    output_dir = os.path.join(root_path, "wumpus_game")
    os.makedirs(output_dir, exist_ok=True)
  
    # ----------------------------------------------------
    # 1. GENERAZIONE FILE: agent0.ltlf 
    # ----------------------------------------------------
    # Sub-specification: Fcollect
    fcollect_parts = [f"F(at{r}{c})" for r, c in gold_positions]
    fcollect = " && ".join(fcollect_parts)
  
    # Sub-specification: Ftask
    ftask_pure = f"E(F(({fcollect})))"

    # Sub-specification: Fmutex
    fmutex_or = " || ".join(agent_actions)
    fmutex_imp_parts = []
    for i, act in enumerate(agent_actions):
        other_actions = [f"!{other}" for j, other in enumerate(agent_actions) if i != j]
        others_combined = " && ".join(other_actions)
        fmutex_imp_parts.append(f"({act} -> ({others_combined}))")
    fmutex_imp = " && ".join(fmutex_imp_parts) 
    fmutex_pure = f"A(G({fmutex_or} && {fmutex_imp}))"


    # Sub-specification: Favoid_wumpus
    w1_collision_cells = [f"(at{r}{c} && w1at{r}{c})" for r in range(rows) for c in range(cols)]
    w1_all_collisions_or = " || ".join(w1_collision_cells)
    favoid1_pure = f"A(G(!({w1_all_collisions_or})))"
    # Sub-specification: Favoid_wumpus
    w2_collision_cells = [f"(at{r}{c} && w2at{r}{c})" for r in range(rows) for c in range(cols)]
    w2_all_collisions_or = " || ".join(w2_collision_cells)
    favoid2_pure = f"A(G(!({w2_all_collisions_or})))"
  
    f0_pure = f"{ftask_pure} && {fmutex_pure} && {favoid1_pure} && {favoid2_pure}"
    
  
    with open(os.path.join(output_dir, "agent0.ltlf"), "w") as f:
        f.write(f0_pure)
    print("File 'agent0.ltlf' salvato con successo.")


    # ----------------------------------------------------
    # 2. GENERAZIONE FILE: agent1.ltlf (Wumpus)
    # ----------------------------------------------------
    wumpus1_cells = [f"w1at{r}{cols-1}" for r in range(rows)]
    fwumpus1_pure = "A(G(" + " || ".join(wumpus1_cells) + "))"


    fmutex_w1_or = " || ".join(wumpus1_actions)
    fmutex_w1_imp_parts = []
    for i, act in enumerate(wumpus1_actions):
        other_wactions = [f"!{other}" for j, other in enumerate(wumpus1_actions) if i != j]
        others_wcombined = " && ".join(other_wactions)
        fmutex_w1_imp_parts.append(f"({act} -> ({others_wcombined}))")
    fmutex_w1_imp = " && ".join(fmutex_w1_imp_parts)

    c_idx = cols - 1
    patrol_clauses = []
    for r in range(rows):
        current_var = f"w1at{r}{c_idx}"
        if r == 0:
            next_vars = f"w1at{r+1}{c_idx}"
        elif r == rows - 1:
            next_vars = f"w1at{r-1}{c_idx}"
        else:
            next_vars = f"w1at{r-1}{c_idx} || w1at{r+1}{c_idx}"
        patrol_clauses.append(f"({current_var} -> X({next_vars}))")
    w_patrol_spec = "A(G(" + " && ".join(patrol_clauses) + "))"

    
    fmutex_w1 = f"A(G({fmutex_w1_or} && {fmutex_w1_imp})) && {w_patrol_spec}"

    f1_pure = f"{fwumpus1_pure} && {fmutex_w1}"
    with open(os.path.join(output_dir, "agent1.ltlf"), "w") as f:
        f.write(f1_pure)
    print("File 'agent1.ltlf' saved.")

     # ----------------------------------------------------
    # 2. GENERAZIONE FILE: agent2.ltlf (Wumpus)
    # ----------------------------------------------------
    #it can be only in the last row
    wumpus2_cells = [f"w2at{rows-1}{c}" for c in range(cols)]
    fwumpus2_pure = "A(G(" + " || ".join(wumpus2_cells) + "))"


    fmutex_w2_or = " || ".join(wumpus2_actions)
    fmutex_w2_imp_parts = []
    for i, act in enumerate(wumpus2_actions):
        other_wactions = [f"!{other}" for j, other in enumerate(wumpus2_actions) if i != j]
        others_wcombined = " && ".join(other_wactions)
        fmutex_w2_imp_parts.append(f"({act} -> ({others_wcombined}))")
    fmutex_w2_imp = " && ".join(fmutex_w2_imp_parts)

    r_idx = rows - 1
    patrol_clauses = []
    for c in range(cols):
        current_var = f"w2at{r_idx}{c}"
        if c == 0:
            next_vars = f"w2at{r_idx}{c+1}"
        elif c == cols - 1:
            next_vars = f"w2at{r_idx}{c-1}"
        else:
            next_vars = f"w2at{r_idx}{c-1} || w2at{r_idx}{c+1}"
        patrol_clauses.append(f"({current_var} -> X({next_vars}))")
    w_patrol_spec = "A(G(" + " && ".join(patrol_clauses) + "))"

    
    fmutex_w2 = f"A(G({fmutex_w2_or} && {fmutex_w2_imp})) && {w_patrol_spec}"

    f2_pure = f"{fwumpus2_pure} && {fmutex_w2}"
    with open(os.path.join(output_dir, "agent2.ltlf"), "w") as f:
        f.write(f2_pure)
    print("File 'agent2.ltlf' saved.")


    # ----------------------------------------------------
    # 3. GENERAZIONE FILE: env.ltlf 
    # ----------------------------------------------------
    def get_grid_state_str(target_r, target_c, max_rows, max_cols, prefix):
        state_parts = []
        for r in range(max_rows):
            for c in range(max_cols):
                if r == target_r and c == target_c:
                    state_parts.append(f"{prefix}{r}{c}")
                else:
                    state_parts.append(f"!{prefix}{r}{c}")
        return " && ".join(state_parts)

    #Initial states for agent and wumpus
    init_a_parts = []
    init_w1_parts = []
    init_w2_parts = []
    for r in range(rows):
        for c in range(cols):
            if (r, c) == agent_start:  init_a_parts.append(f"at{r}{c}")
            else:                      init_a_parts.append(f"!at{r}{c}")
                
            if (r, c) == wumpus1_start: init_w1_parts.append(f"w1at{r}{c}")
            else:                      init_w1_parts.append(f"!w1at{r}{c}")

            if (r, c) == wumpus2_start: init_w2_parts.append(f"w2at{r}{c}")
            else:                      init_w2_parts.append(f"!w2at{r}{c}")
    init_agent_str = " && ".join(init_a_parts)
    init_wumpus1_str = " && ".join(init_w1_parts)
    init_wumpus2_str = " && ".join(init_w2_parts)

    # mutex body for agent and wumpus
    a_or = "(" + " || ".join(agent_actions) + ")"
    a_imp_list = []
    for i, act in enumerate(agent_actions):
        others = [f"!{o}" for j, o in enumerate(agent_actions) if i != j]
        a_imp_list.append(f"({act} -> ({' && '.join(others)}))")

    mutex_body_agent = f"{a_or} && " + " && ".join(a_imp_list)

    w1_or = "(" + " || ".join(wumpus1_actions) + ")"
    w1_imp_list = []
    for i, act in enumerate(wumpus1_actions):
        others = [f"!{o}" for j, o in enumerate(wumpus1_actions) if i != j]
        w1_imp_list.append(f"({act} -> ({' && '.join(others)}))")

    mutex_body_wumpus1 = f"{w1_or} && " + " && ".join(w1_imp_list)

    w2_or = "(" + " || ".join(wumpus2_actions) + ")"
    w2_imp_list = []
    for i, act in enumerate(wumpus2_actions):
        others = [f"!{o}" for j, o in enumerate(wumpus2_actions) if i != j]
        w2_imp_list.append(f"({act} -> ({' && '.join(others)}))")

    mutex_body_wumpus2 = f"{w2_or} && " + " && ".join(w2_imp_list)

    #agent transitions
    etrans_list_agent = []
    for r in range(rows):
        for c in range(cols):
            for act in agent_actions:
                next_r, next_c = r, c
                if "north" in act or "nord" in act:
                    if r > 0: next_r = r - 1
                elif "south" in act or "sud" in act:
                    if r < rows - 1: next_r = r + 1
                elif "west" in act or "ovest" in act:
                    if c > 0: next_c = c - 1
                elif "east" in act or "est" in act:
                    if c < cols - 1: next_c = c + 1
                
                next_state = get_grid_state_str(next_r, next_c, rows, cols, "at")
                etrans_list_agent.append(f"((at{r}{c} && {act}) -> X({next_state}))")
    a_etrans_pure = " && ".join(etrans_list_agent)

    # wumpus transitions
    etrans_list_wumpus1 = []
    for r in range(rows):
        for c in range(cols):
            for act in wumpus1_actions:
                next_r, next_c = r, c
                if "north" in act or "nord" in act:
                    if r > 0: next_r = r - 1
                elif "south" in act or "sud" in act:
                    if r < rows - 1: next_r = r + 1
                elif "west" in act or "ovest" in act:
                    if c > 0: next_c = c - 1
                elif "east" in act or "est" in act:
                    if c < cols - 1: next_c = c + 1
                
                next_state = get_grid_state_str(next_r, next_c, rows, cols, "w1at")
                etrans_list_wumpus1.append(f"((w1at{r}{c} && {act}) -> X({next_state}))")
    w1_etrans_pure = " && ".join(etrans_list_wumpus1)

    # wumpus transitions
    etrans_list_wumpus2 = []
    for r in range(rows):
        for c in range(cols):
            for act in wumpus2_actions:
                next_r, next_c = r, c
                if "north" in act or "nord" in act:
                    if r > 0: next_r = r - 1
                elif "south" in act or "sud" in act:
                    if r < rows - 1: next_r = r + 1
                elif "west" in act or "ovest" in act:
                    if c > 0: next_c = c - 1
                elif "east" in act or "est" in act:
                    if c < cols - 1: next_c = c + 1
                
                next_state = get_grid_state_str(next_r, next_c, rows, cols, "w2at")
                etrans_list_wumpus2.append(f"((w2at{r}{c} && {act}) -> X({next_state}))")
    w2_etrans_pure = " && ".join(etrans_list_wumpus2)

    agent_block = f"A(G({mutex_body_agent}) -> (({init_agent_str}) && G({a_etrans_pure})))"
    wumpus1_block = f"A(G({mutex_body_wumpus1}) -> (({init_wumpus1_str}) && G({w1_etrans_pure})))"
    wumpus2_block = f"A(G({mutex_body_wumpus2}) -> (({init_wumpus2_str}) && G({w2_etrans_pure})))"
    fenv_pure = f"({agent_block}) && ({wumpus1_block}) && ({wumpus2_block})"

    with open(os.path.join(output_dir, "env.ltlf"), "w") as f:
        f.write(fenv_pure)
    print("File 'env.ltlf' saved.")

    # ----------------------------------------------------
    # 4. GENERAZIONE FILE: var.part
    # ----------------------------------------------------
    part_lines = []
    agent0_vars = [f"at{r}{c}" for r in range(rows) for c in range(cols)]
    agent1_vars = [f"w1at{r}{c}" for r in range(rows) for c in range(cols)]
    agent2_vars = [f"w2at{r}{c}" for r in range(rows) for c in range(cols)]
    
    #part_lines.append(".inputs: " + " ".join(agent0_vars))  
    part_lines.append(".inputs: " + " ".join(agent0_vars) + " " + " ".join(agent1_vars)+ " " + " ".join(agent2_vars))  
    part_lines.append(".agent0: " + " ".join(agent_actions))
    part_lines.append(".agent1: " + " ".join(wumpus1_actions))
    part_lines.append(".agent2: " + " ".join(wumpus2_actions))
  
    part_path = os.path.join(output_dir, "var.part")
    with open(part_path, "w") as f:
        f.write("\n".join(part_lines))
    print("File 'var.part' saved.")


if __name__ == "__main__":
    generate_final_specs_and_partition()

