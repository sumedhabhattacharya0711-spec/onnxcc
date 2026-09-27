from pathlib import Path
import numpy as np
import onnx
from onnx import helper, TensorProto, numpy_helper

SEED = 42
OPSET = 17
IR_VERSION = 8

OUT_DIR = Path(__file__).resolve().parent
MODEL_PATH = OUT_DIR / "mlp.onnx"
INPUT_PATH = OUT_DIR / "mlp_input.bin"


def make_weights(rng: np.random.Generator) -> dict[str, np.ndarray]:
    return {
        "W1": rng.standard_normal((4, 8)).astype(np.float32),
        "b1": rng.standard_normal((8,)).astype(np.float32),
        "W2": rng.standard_normal((8, 2)).astype(np.float32),
        "b2": rng.standard_normal((2,)).astype(np.float32),
    }


def build_model(weights: dict[str, np.ndarray]) -> onnx.ModelProto:
    nodes = [
        helper.make_node("MatMul", ["x", "W1"], ["mm1"], name="matmul1"),
        helper.make_node("Add", ["mm1", "b1"], ["h1"], name="add1"),
        helper.make_node("Relu", ["h1"], ["a1"], name="relu1"),
        helper.make_node("MatMul", ["a1", "W2"], ["mm2"], name="matmul2"),
        helper.make_node("Add", ["mm2", "b2"], ["h2"], name="add2"),
        helper.make_node("Relu", ["h2"], ["y"], name="relu2"),
    ]
    initializers = [numpy_helper.from_array(arr, name) for name, arr in weights.items()]
    graph = helper.make_graph(
        nodes,
        "mlp_4_8_2",
        inputs=[helper.make_tensor_value_info("x", TensorProto.FLOAT, [1, 4])],
        outputs=[helper.make_tensor_value_info("y", TensorProto.FLOAT, [1, 2])],
        initializer=initializers,
    )
    model = helper.make_model(
        graph,
        opset_imports=[helper.make_operatorsetid("", OPSET)],
        producer_name="onnxcc-fixtures",
    )
    model.ir_version = IR_VERSION
    onnx.checker.check_model(model, full_check=True)
    return model


def reference_forward(x: np.ndarray, weights: dict[str, np.ndarray]) -> np.ndarray:
    a1 = np.maximum(x @ weights["W1"] + weights["b1"], 0)
    return np.maximum(a1 @ weights["W2"] + weights["b2"], 0)


def main() -> None:
    rng = np.random.default_rng(SEED)
    weights = make_weights(rng)
    x = rng.standard_normal((1, 4)).astype(np.float32)

    model = build_model(weights)
    onnx.save(model, MODEL_PATH)

    INPUT_PATH.write_bytes(x.astype("<f4").tobytes())

    assert INPUT_PATH.stat().st_size == 16, "input must be exactly 16 bytes"

    print(f"wrote {MODEL_PATH.name}: {len(model.graph.node)} nodes, "
          f"{len(model.graph.initializer)} initializers")
    print(f"wrote {INPUT_PATH.name}: {INPUT_PATH.stat().st_size} bytes")
    print("input:          ", x)
    print("expected output:", reference_forward(x, weights))


if __name__ == "__main__":
    main()